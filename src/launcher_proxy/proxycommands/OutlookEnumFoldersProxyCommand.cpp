#include "OutlookEnumFoldersProxyCommand.h"
#include <windows.h>
#include "StringUtil.h"
#include "AutoWrap.h"
#include <atlbase.h>
#include <deque>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace launcherproxy { 


struct OutlookEnumFoldersProxyCommand::PImpl
{
	bool GetRunningApp(CComPtr<IDispatch>& app);
};

// 起動中のOutlook.Applicationインスタンスを取得する
bool OutlookEnumFoldersProxyCommand::PImpl::GetRunningApp(CComPtr<IDispatch>& app)
{
	// OutlookのCLSIDを取得
	CLSID CLSID_Outlook;
	HRESULT hr = CLSIDFromProgID(L"Outlook.Application", &CLSID_Outlook);
	if (FAILED(hr)) {
		spdlog::debug("Outlook is not installed.");
		return false;
	}

	// 起動中のOutlook.Applicationインスタンスを取得する
	CComPtr<IUnknown> unk;
	hr = GetActiveObject(CLSID_Outlook, nullptr, &unk);
	if (FAILED(hr)) {
		spdlog::debug("Failed to get outlook instance\n");
		return false;
	}

	hr = unk->QueryInterface(IID_IDispatch, (void**)&app);
	return !FAILED(hr);
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



REGISTER_PROXYCOMMAND(OutlookEnumFoldersProxyCommand)

OutlookEnumFoldersProxyCommand::OutlookEnumFoldersProxyCommand() : in(new PImpl)
{
}

OutlookEnumFoldersProxyCommand::~OutlookEnumFoldersProxyCommand()
{
}

std::string OutlookEnumFoldersProxyCommand::GetName()
{
	return "outlook_enumfolders";
}

static bool GetFolderName(CComPtr<IDispatch>& folder, std::wstring& name)
{
	// 現在のフォルダ名を取得
	CComVariant result;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, folder, (LPOLESTR)L"Name", 0);
	if (FAILED(hr)) {
		return false;
	}

	name = (LPCWSTR)result.bstrVal;
	return true;
}

// フォルダ内のメール件数を取得
static int GetItemCount(CComPtr<IDispatch>& folder)
{
	CComVariant result;

	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, folder, (LPOLESTR)L"Items", 0);
	if (FAILED(hr)) {
		return 0;
	}
	CComPtr<IDispatch> items(result.pdispVal);
	CComVariant itemCount;
	hr = AutoWrap(DISPATCH_PROPERTYGET, &itemCount, items, (LPOLESTR)L"Count", 0);
	return FAILED(hr) ? 0 : itemCount.intVal;
}

// サブフォルダの数を取得
static int GetSubFolderCount(CComPtr<IDispatch>& folders)
{
	CComVariant count;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &count, folders, (LPOLESTR)L"Count", 0);
	return FAILED(hr) ? 0 : count.intVal;
}

static bool GetParentFolder(CComPtr<IDispatch>& curFolder, CComPtr<IDispatch>& parentFolder)
{
	CComVariant result;
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, curFolder, (LPOLESTR)L"Parent", 0);
	if  (FAILED(hr)) {
		return false;
	}

	parentFolder = CComPtr<IDispatch>(result.pdispVal);
	return true;
}


bool OutlookEnumFoldersProxyCommand::Execute(json& json_req, json& json_res)
{
	// 起動中のApplicaitonを取得
	CComPtr<IDispatch> app;
	if (in->GetRunningApp(app) == false) {
		return false;
	}

	CComVariant result;

	CComVariant arg1(L"MAPI");
	HRESULT hr = AutoWrap(DISPATCH_METHOD, &result, app, (LPOLESTR)L"GetNameSpace", 1, &arg1);
	if (FAILED(hr)) {
		spdlog::error("Failed to GetNameSpace.");
		return false;
	}
	CComPtr<IDispatch> mapi(result.pdispVal);

	// Inboxを得る
	arg1 = CComVariant(6);
	hr = AutoWrap(DISPATCH_METHOD, &result, mapi, (LPOLESTR)L"GetDefaultFolder", 1, &arg1);
	if (FAILED(hr)) {
		spdlog::error("Failed to get Inbox folder.");
		return false;
	}
	CComPtr<IDispatch> inboxFolder(result.pdispVal);


	// Inboxの一階層上のフォルダを取得する
	CComPtr<IDispatch> rootFolder;
	GetParentFolder(inboxFolder, rootFolder);

	struct STACK_ITEM {
		std::wstring mParentPath;
		CComPtr<IDispatch> mFolder;
	};

	auto folders = json::array();

	// 深さ優先探索をする
	std::deque<STACK_ITEM> stk;
	stk.push_front(STACK_ITEM{ _T(""), rootFolder});

	std::string tmp;

	while(stk.empty() == false) {

		// スタックから今回の走査対象の要素を取りだす
		auto item = stk.front();
		stk.pop_front();

		// 親階層のフォルダ名と現在のフォルダ
		const auto& parentPath = item.mParentPath;
		CComPtr<IDispatch>& curFolder = item.mFolder;

		// 現在のフォルダ名を取得
		std::wstring name;
		if (curFolder != rootFolder && GetFolderName(curFolder, name) == false) {
			continue;
		}

		std::wstring fullName(parentPath.empty() ? name : (parentPath + L"/" + name));

		// EntryIDを取得
		hr = AutoWrap(DISPATCH_PROPERTYGET, &result, curFolder, (LPOLESTR)L"EntryID", 0);
		if (FAILED(hr)) {
			continue;
		}
		std::wstring entryID = (LPCWSTR)result.bstrVal;

		if (curFolder != rootFolder) {
			int itemCount = GetItemCount(curFolder);
			if (itemCount > 0) {

				json folder = {};
				folder["full_name"] = utf2utf(fullName, tmp);
				folder["item_count"] = itemCount;
				folder["entry_id"] = utf2utf(entryID, tmp);
				folders.push_back(folder);
			}
		}

		// サブフォルダからみた親階層を表す文字列
		std::wstring parent = fullName;

		// サブフォルダの数を取得
		hr = AutoWrap(DISPATCH_PROPERTYGET, &result, curFolder, (LPOLESTR)L"Folders", 0);
		if (FAILED(hr)) {
			continue;
		}
		CComPtr<IDispatch> subFolders(result.pdispVal);

		int subFolderCount = GetSubFolderCount(subFolders);
		for (int i = subFolderCount; i >= 1; --i) {

			arg1 = CComVariant(i);
			hr = AutoWrap(DISPATCH_METHOD, &result, subFolders, (LPOLESTR)L"Item", 1, &arg1);
			if (FAILED(hr)) {
				continue;
			}

			CComPtr<IDispatch> subFolder(result.pdispVal);
			hr = AutoWrap(DISPATCH_METHOD, &result, subFolder, (LPOLESTR)L"DefaultItemType", 0);
			if (FAILED(hr) || result.intVal != 0) { 
				continue;
			}

			stk.push_front(STACK_ITEM{parent, subFolder});
		}
	}

	json_res["result"] = true;
	json_res["folders"] = folders;
	return true;
}

} //"sheet does not found."end of namespace 



