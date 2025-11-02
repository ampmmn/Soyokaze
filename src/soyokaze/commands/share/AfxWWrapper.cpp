#include "pch.h"
#include "AfxWWrapper.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"

using NormalPriviledgeProcessProxy = launcherapp::processproxy::NormalPriviledgeProcessProxy;

using json = nlohmann::json;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// コンストラクタ
AfxWWrapper::AfxWWrapper()
{
}

// デストラクタ
AfxWWrapper::~AfxWWrapper()
{
}

/**
  自窓のディレクトリパスを取得
 	@param curDir 自窓のディレクトリパス
*/
bool AfxWWrapper::GetCurrentDir(std::wstring& curDir)
{
	// 管理者権限でアプリを実行していると、通常権限で実行しているあふwのAPI呼び出しに失敗する
	// launcher_proxy.exeを経由することにより、常に通常権限で実行されるようにする。

	json json_req;
	json_req["command"] = "getcurrentafxwdir";

	// リクエストを送信する
	json json_res;

	auto proxy = NormalPriviledgeProcessProxy::GetInstance();
	if (proxy->SendRequest(json_req, json_res) == false) {
		return false;
	}
	
	// 結果を取得
	if (json_res.find("path") == json_res.end()) {
		spdlog::error("unexpected response.");
		return false;
	}

	UTF2UTF((const std::string)json_res["path"], curDir);
	return true;

}

// 自窓のカレントディレクトリを移動
bool AfxWWrapper::SetCurrentDir(const std::wstring& path)
{
	// 管理者権限でアプリを実行していると、通常権限で実行しているあふwのAPI呼び出しに失敗する
	// launcher_proxy.exeを経由することにより、常に通常権限で実行されるようにする。

	std::string dst;

	json json_req;
	json_req["command"] = "setcurrentafxwdir";
	json_req["path"] = UTF2UTF(path, dst);

	// リクエストを送信する
	json json_res;

	auto proxy = NormalPriviledgeProcessProxy::GetInstance();
	return proxy->SendRequest(json_req, json_res);
}

// あふの自窓の選択ファイルパスを取得
bool AfxWWrapper::GetSelectionPath(std::wstring& path, int index)
{
	json json_req;
	json_req["command"] = "getafxselectionpath";
	json_req["index"] = index;

	// リクエストを送信する
	auto proxy = NormalPriviledgeProcessProxy::GetInstance();

	json json_res;
	if (proxy->SendRequest(json_req, json_res) == false) {
		return false;
	}
	
	// 結果を取得
	if (json_res.find("items") == json_res.end()) {
		spdlog::error("unexpected response.");
		return false;
	}
	std::wstring tmp;

	auto items = json_res["items"];
	for (auto& item : items) {
		auto& ret_path = UTF2UTF(item["path"].get<std::string>(), tmp);

		path += path.empty() ? _T("\"") : _T(" \"");
		path += ret_path;
		path += _T("\"");
	}

	return true;
}


