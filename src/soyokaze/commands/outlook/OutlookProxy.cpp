#include "pch.h"
#include "OutlookProxy.h"
#include "commands/common/AutoWrap.h"
#include "utility/ScopeAttachThreadInput.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"
#include <deque>

using namespace launcherapp::commands::common;
using json = nlohmann::json;
using NormalPriviledgeProcessProxy = launcherapp::processproxy::NormalPriviledgeProcessProxy;

namespace launcherapp { namespace commands { namespace outlook {

OutlookProxy::OutlookProxy()
{
}

OutlookProxy::~OutlookProxy()
{
}

OutlookProxy* OutlookProxy::GetInstance()
{
	static OutlookProxy inst;
	return &inst;
}

bool OutlookProxy::Initialize()
{
	return true;
}

bool OutlookProxy::IsAppRunning()
{
	json json_req;
	json_req["command"] = "outlook_isapprunning";

	auto proxy = NormalPriviledgeProcessProxy::GetInstance();

	// リクエストを送信する
	json json_res;
	if (proxy->SendRequest(json_req, json_res) == false) {
		return false;
	}
	
	return json_res["result"] != false;
}

// パターンで指定した検索条件に合致するフォルダの一覧の取得する
bool OutlookProxy::EnumFolders(std::vector<QueryResult>& results)
{
	json json_req;
	json_req["command"] = "outlook_enumfolders";

	auto proxy = NormalPriviledgeProcessProxy::GetInstance();

	// リクエストを送信する
	json json_res;
	if (proxy->SendRequest(json_req, json_res) == false) {
		return false;
	}

	if (json_res["result"] == false) {
		return false;
	}

	std::vector<QueryResult> tmpResults;

	auto folders = json_res["folders"];
	for (auto folder : folders) {
		QueryResult result;
		UTF2UTF(folder["full_name"].get<std::string>(), result.mFullName);
		result.mItemCount = folder["item_count"].get<int>();
		UTF2UTF(folder["entry_id"].get<std::string>(), result.mEntryID);

		tmpResults.push_back(result);
	}

	results.swap(tmpResults);
	return true;
}

// Outlookで表示中のフォルダを変更する
bool OutlookProxy::SelectFolder(const CString& entryID)
{
	std::string dst;

	json json_req;
	json_req["command"] = "outlook_selectfolder";
	json_req["entry_id"] = UTF2UTF(entryID, dst);

	auto proxy = NormalPriviledgeProcessProxy::GetInstance();

	// リクエストを送信する
	json json_res;
	if (proxy->SendRequest(json_req, json_res) == false) {
		return false;
	}

	return json_res["result"] != false;
}
	


}}}
