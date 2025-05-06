#include "pch.h"
#include "RemoteServer.h"
#include "remote/ServerPipe.h"
#include "remote/RemoteServerSetting.h"
#include "remote/CommandQueryReceiver.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandRepository.h"

#include <mutex>
#include <thread>

#pragma warning( push )
#pragma warning( disable : 26800 26819 )
#include <nlohmann/json.hpp>
#pragma warning( pop )

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp { namespace remote {

using json = nlohmann::json;
using CommandRepository = launcherapp::core::CommandRepository;
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;

struct RemoteServer::PImpl :
	public AppPreferenceListenerIF
{
	PImpl()
 	{
		// アプリケーションの設定変更リスナーとして登録
		AppPreference::Get()->RegisterListener(this);
	}
	~PImpl()
 	{
		// アプリケーションの設定変更リスナーから解除
		AppPreference::Get()->UnregisterListener(this);
	}

	/**
	 * @brief アプリケーションの初回起動時に呼ばれる
	 */
	void OnAppFirstBoot() override {}

	/**
	 * @brief アプリケーションの通常起動時に呼ばれる
	 */
	void OnAppNormalBoot() override {}

	/**
	 * @brief アプリケーションの設定が更新されたときに呼ばれる
	 */
	void OnAppPreferenceUpdated() override
	{
		// いったん止める
		Stop();

		RemoteServerSetting serverFunction;
		if (serverFunction.IsEnable()) {
			Start();
		}
	}

	/**
	 * @brief アプリケーションの終了時に呼ばれる
	 */
	void OnAppExit() override {
		Stop();
	}

	void Start()
	{
		// 待ち受けスレッド終了検知用のイベント
		mAbortEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		mIsAbort = false;

		// 待ち受けスレッド実行
		std::thread th([&]() { Run(); });
		th.detach();
	}

	void Stop()
	{
		if (mPipe.IsOpen() == false) {
			spdlog::info("[remote] server thread is already down.");
			return ;
		}
		// 待ち受けスレッド終了検知用のイベント
		ResetEvent(mAbortEvent);

		// 終了フラグ立てる
		{
			std::lock_guard<std::mutex> lock(mMutex);
			mIsAbort = true;
		}

		// 待ち受けスレッドの終了を待つ
		spdlog::info("[remote]waiting for exit of server thread.");
		DWORD result = WaitForSingleObject(mAbortEvent, 5000);
		if (result == WAIT_TIMEOUT) {
			spdlog::warn("[remote]wait timeout.");
		}
		else {
			spdlog::info("[remote]completed waiting.");
		}
		CloseHandle(mAbortEvent);
		mAbortEvent = nullptr;
	}

	void Run();
	bool ProcessRequest(std::string& readBuffer, json& json_res);
	void EnumCommand(json& json_req, json& json_res);
	void RunCommand(json& json_req, json& json_res);

	bool IsAbort()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsAbort;
	}

	ServerPipe mPipe;
	HANDLE mPipeHandle{nullptr};
	HANDLE mAbortEvent{nullptr};
	std::mutex mMutex;
	bool mIsAbort{false};
	QueryReceiver mQueryReceiver;
};

void RemoteServer::PImpl::Run()
{
	spdlog::info("[remote]Start server thread id:{}", GetCurrentThreadId());

	// クライアント側の要求を受けるための名前付きパイプを作成する
	if (mPipe.Open() == false) {
		spdlog::error("[remote]Failed to create named pipe.");
		return ;
	}

	// クライアントからの要求を待ち受ける
	std::string buffer;
	while(IsAbort() == false) {

		// リクエストを待つ
		if (mPipe.WaitForRequest(buffer) == false) {
			Sleep(50);
			continue;
		}

		// リクエストを処理する
		json json_res;
		if (ProcessRequest(buffer, json_res) == false) {
			continue;
		}
		// リクエストに対するレスポンスを返す
		mPipe.SendResponse(json_res);
	}

	spdlog::info("[remote]Leaving thread id:{}", GetCurrentThreadId());

	// 名前付きパイプを閉じる
	mPipe.Close();

	// 停止したことを伝える
	SetEvent(mAbortEvent);
}

/**
 * @brief リクエストを処理する
 * @param readBuffer リクエストデータ
 * @param json_res 応答用のJSONデータ
 * @return true: 処理した、false: 読みかけ
 */
bool RemoteServer::PImpl::ProcessRequest(std::string& readBuffer, json& json_res)
{
	auto pos = readBuffer.find('\n');
	if (pos == std::string::npos) {
		// 読みかけなので処理しない
		return false;
	}

	// JSONデータを取り出す。処理した後はバッファからカットする
	auto request = readBuffer.substr(0, pos);
	readBuffer.erase(0, pos + 1);

	try {
		// 受け取ったJSONデータを解析
		json json_req = json::parse(request);
		if (json_req.find("command") == json_req.end()) {
			// 不正なリクエスト
			spdlog::warn("[remote] command key does not exist.");
			return false;
		}

		// コマンド名に応じた処理をする
		std::string command_name = json_req["command"];
		if (command_name == "ls") {
			EnumCommand(json_req, json_res);
			return true;
		}
		else if (command_name == "run") {
			RunCommand(json_req, json_res);
			return true;
		}

		json_res["result"] = false;
		json_res["reason"] = "Unknown command.";
		return true;
	}
	catch(...) {
		spdlog::error("[remote] An exception ocurred.");
		json_res["result"] = false;
		json_res["reason"] = "An exception ocurred";
		return true;
	}
}

void RemoteServer::PImpl::EnumCommand(json& json_req, json& json_res)
{
	if (json_req.find("query") == json_req.end()) {
		json_res["result"] = false;
		return;
	}

	int limit = 32;
	if (json_req.find("limit") != json_req.end()) {
		limit = json_res["limit"].get<int>();
	}

	std::wstring dst;
	auto& queryString = UTF2UTF(json_req["query"], dst);

	// キーワードによる絞り込みを実施し、結果が返るまで待つ
	std::vector<launcherapp::core::Command*> commands;
	mQueryReceiver.QuerySync(queryString, commands);

	// 結果を取得
	int itemCount = 0;

	json items = json::array();
	std::string tmp;
	for (auto cmd : commands) {

		if (itemCount < limit) {
			json item = json::object();
			item["name"] = UTF2UTF(cmd->GetName(), tmp);
			item["description"] =UTF2UTF(cmd->GetDescription(), tmp); 
			item["type_name"] =UTF2UTF(cmd->GetTypeDisplayName(), tmp); 
			// ToDo: アイコンも扱えるようにしたい

			items.push_back(item);
			itemCount++;
		}

		cmd->Release();


	}

	// 応答用のJSONに結果をいれる
	json_res["result"] = true;
	json_res["item_count"] = itemCount;
	json_res["items"] = items;
}

void RemoteServer::PImpl::RunCommand(json& json_req, json& json_res)
{
	// 必須パラメータがない場合はエラー扱い
	if (json_req.find("query") == json_req.end()) {
		json_res["result"] = false;
		json_res["reason"] = "parmeter 'query' must be exist.";
		return;
	}
	if (json_req.find("index") == json_req.end()) {
		json_res["result"] = false;
		json_res["reason"] = "parmeter 'index' must be exist.";
		return;
	}

	// JSONからパラメータを取り出す
	std::wstring dst;
	auto& queryString = UTF2UTF(json_req["query"], dst);
	int index = json_req["index"].get<int>();

	// キーワードによる絞り込みを実施し、結果が返るまで待つ
	std::vector<launcherapp::core::Command*> commands;
	mQueryReceiver.QuerySync(queryString, commands);

	if (index < -1 || commands.size() <= (size_t)index) {
		json_res["result"] = true;
		json_res["reason"] = "parmeter 'index' is out of bounds.";
		json_res["is_found_command"] = false;
		return;
	}

	auto cmd = commands[index];
	auto commandParam = CommandParameterBuilder::Create(queryString.c_str());

	// コマンドを実行する
	std::thread th([cmd, commandParam]() {
		cmd->Execute(commandParam);
		commandParam->Release();
		cmd->Release();
	});
	th.detach();

	// 応答用のJSONに結果をいれる
	json_res["result"] = true;
	json_res["is_found_command"] = true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


RemoteServer::RemoteServer() : in(new PImpl)
{
}

RemoteServer::~RemoteServer()
{
	in->Stop();
}

void RemoteServer::Start()
{
	RemoteServerSetting serverFunction;
	if (serverFunction.IsEnable()) {
		in->Start();
	}
}


}} // end of namespace launcherapp::remote
