#include "pch.h"
#include "RemoteCommandLauncher.h"
#include "commands/remote/RemoteAdhocCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "mainwindow/LocalCommandLauncher.h"
#include "utility/ProcessPath.h"
#include "resource.h"
#include <thread>
#include <nlohmann/json.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;
using json = nlohmann::json;

namespace launcherapp { namespace commands { namespace remote {


RemoteCommandLauncher::RemoteCommandLauncher(RemoteClient* client) :
	mClient(client)
{
}

RemoteCommandLauncher::~RemoteCommandLauncher()
{
}

// 読み込み
bool RemoteCommandLauncher::Load()
{
	return true;
}

// 検索リクエスト実施
void RemoteCommandLauncher::Query(const launcherapp::commands::core::CommandQueryRequest& req)
{
	// 検索文字列を取得
	auto parameter = req.GetCommandParameter();
	auto wholeString = CString(parameter->GetWholeString());

	// サーバに送るリクエストデータを生成
	std::string dst;
	json json_req = json::object();
	json_req["command"] = "ls";
	json_req["query"] = UTF2UTF(wholeString, dst);

	// リクエストを送る
	if (mClient->SendRequest(json_req) == false) {
		return ;
	}
	// レスポンスを受け取る
	json json_res = json::object();
	if (mClient->ReceiveResponse(json_res) == false) {
		return;
	}

	// 結果を取り出し、成否を確認する
	if (json_res.find("result") == json_res.end()) {
		return;
	}
	if (json_res["result"].get<bool>() == false) {
		auto reason = json_res["reason"].get<std::string>();
		spdlog::error("[remote] server returned error err:{}", reason);
		return;
	}

	std::vector<launcherapp::core::Command*>* items = new std::vector<launcherapp::core::Command*>();

	// 一覧を取得し、AdhocCommandとして登録する
	std::wstring tmp;
	auto json_items = json_res["items"];
	int index = 0;
	for (auto item : json_items) {
		std::wstring name = UTF2UTF(item["name"].get<std::string>(), tmp);
		std::wstring description = UTF2UTF(item["description"].get<std::string>(), tmp);
		std::wstring typeName = UTF2UTF(item["type_name"].get<std::string>(), tmp);

		auto command =
		 	RemoteAdhocCommand::CreateInstance(mClient, name.c_str(), description.c_str(), typeName.c_str(), wholeString, index++);
		items->push_back(command);
	}

	PostMessage(req.GetNotifyWindow(), req.GetNotifyMessage(), 0, (LPARAM)items);
}

// コマンド実行
bool RemoteCommandLauncher::Execute(const CString& str)
{
	// RemoteCommandLauncherとして特別な処理は実施しないため、本来の処理を呼ぶ
	return launcherapp::mainwindow::LocalCommandLauncher::CallExecute(str);
}

// ファイルがドロップされた
void RemoteCommandLauncher::DropFiles(const std::vector<CString>& files)
{
	// RemoteCommandLauncherとして特別な処理は実施しないため、本来の処理を呼ぶ
	launcherapp::mainwindow::LocalCommandLauncher::CallDropFiles(files);
}

// URLがドロップされた
void RemoteCommandLauncher::DropURL(const CString& urlString)
{
	// RemoteCommandLauncherとして特別な処理は実施しないため、本来の処理を呼ぶ
	launcherapp::mainwindow::LocalCommandLauncher::CallDropURL(urlString);
}

// ウインドウ
void RemoteCommandLauncher::CaptureWindow(HWND hwnd)
{
	// RemoteCommandLauncherとして特別な処理は実施しないため、本来の処理を呼ぶ
	launcherapp::mainwindow::LocalCommandLauncher::CallCaptureWindow(hwnd);
}


}}} // end of namespace launcherapp::commands::remote


