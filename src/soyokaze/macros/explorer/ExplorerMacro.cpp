#include "pch.h"
#include "ExplorerMacro.h"
#include "commands/share/AfxWWrapper.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"

using NormalPriviledgeProcessProxy = launcherapp::processproxy::NormalPriviledgeProcessProxy;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using json = nlohmann::json;

namespace launcherapp { namespace macros { namespace builtin {

REGISTER_LAUNCHERMACRO(ExplorerMacro)

ExplorerMacro::ExplorerMacro()
{
	mName = _T("explorer");
}

ExplorerMacro::~ExplorerMacro()
{
}

bool ExplorerMacro::Evaluate(const std::vector<CString>& args, std::vector<CString>& results)
{
	results.clear();

	if (args.size() == 0) {
		return false;
	}

	CString command = args[0];

	if (command.CompareNoCase(_T("location_path")) == 0) {
		return ExpandLocationPath(args, results);
	}
	else if (command.CompareNoCase(_T("selection_path")) == 0) {
		return ExpandSelectionPath(args, results);
	}
	return false;
}

bool ExplorerMacro::ExpandLocationPath(const std::vector<CString>& args, std::vector<CString>& results)
{
	UNREFERENCED_PARAMETER(args);

	// 管理者権限でアプリを実行していると、通常権限で実行しているインスタンスのAPI呼び出しに失敗する
	// launcher_proxy.exeを経由することにより、常に通常権限で実行されるようにする。

	std::string dst;

	json json_req;
	json_req["command"] = "getexplorercurrentdir";

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

	std::wstring ret_path;
	UTF2UTF((const std::string)json_res["path"], ret_path);

	results.emplace_back(ret_path.c_str());

	return true;
}

bool ExplorerMacro::ExpandSelectionPath(const std::vector<CString>& args, std::vector<CString>& results)
{
	int index = -1;
	if (args.size() > 1) {
		index = std::stoi(tstring((LPCTSTR)args[1]));
	}

	// 管理者権限でアプリを実行していると、通常権限で実行しているインスタンスのAPI呼び出しに失敗する
	// launcher_proxy.exeを経由することにより、常に通常権限で実行されるようにする。

	json json_req;
	json_req["command"] = "getexplorerselectiondir";
	json_req["index"] = index;

	auto proxy = NormalPriviledgeProcessProxy::GetInstance();

	// リクエストを送信する
	json json_res;
	if (proxy->SendRequest(json_req, json_res) == false) {
		return false;
	}
	
	// 結果を取得
	if (json_res.find("items") == json_res.end()) {
		spdlog::error("unexpected response.");
		return false;
	}
	auto items = json_res["items"];
	for (auto& item : items) {
		std::wstring tmp;
		auto& ret_path = UTF2UTF(item["path"].get<std::string>(), tmp);
		results.emplace_back(ret_path.c_str());
	}

	return true;
}


}}}
