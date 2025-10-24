#include "pch.h"
#include "OutlookProxy.h"
#include "commands/common/AutoWrap.h"
#include "utility/ScopeAttachThreadInput.h"
#include <deque>

using namespace launcherapp::commands::common;

namespace launcherapp { namespace commands { namespace outlook {

struct OutlookProxy::PImpl
{
	bool GetRunningApp(CComPtr<IDispatch>& app);
};

// 起動中のOutlook.Applicationインスタンスを取得する
bool OutlookProxy::PImpl::GetRunningApp(CComPtr<IDispatch>& app)
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

OutlookProxy::OutlookProxy() : in(new PImpl)
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
	// OutlookのCLSIDを取得
	CLSID CLSID_Outlook;
	HRESULT hr = CLSIDFromProgID(L"Outlook.Application", &CLSID_Outlook);
	if (FAILED(hr)) {
		return false;
	}

	// 起動中のOutlook.Applicationインスタンスを取得する
	CComPtr<IUnknown> unk;
	hr = GetActiveObject(CLSID_Outlook, nullptr, &unk);
	if (FAILED(hr)) {
		return false;
	}
	return true;
}

static bool GetFolderName(CComPtr<IDispatch>& folder, CString& name)
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

// パターンで指定した検索条件に合致するフォルダの一覧の取得する
bool OutlookProxy::EnumFolders(std::vector<QueryResult>& results)
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
		CString mParentPath;
		CComPtr<IDispatch> mFolder;
	};

	// 深さ優先探索をする
	std::deque<STACK_ITEM> stk;
	stk.push_front(STACK_ITEM{ _T(""), rootFolder});

	while(stk.empty() == false) {

		// スタックから今回の走査対象の要素を取りだす
		auto item = stk.front();
		stk.pop_front();

		// 親階層のフォルダ名と現在のフォルダ
		const CString& parentPath = item.mParentPath;
		CComPtr<IDispatch>& curFolder = item.mFolder;

		// 現在のフォルダ名を取得
		CString name;
		if (curFolder != rootFolder && GetFolderName(curFolder, name) == false) {
			continue;
		}

		CString fullName(parentPath.IsEmpty() ? name : (parentPath + _T("/") + name));

		if (curFolder != rootFolder) {
			int itemCount = GetItemCount(curFolder);
			if (itemCount > 0) {
				results.push_back(QueryResult{ fullName, GetItemCount(curFolder), curFolder });
			}
		}

		// サブフォルダからみた親階層を表す文字列
		CString parent = fullName;

		// サブフォルダの数を取得
		hr = AutoWrap(DISPATCH_PROPERTYGET, &result, curFolder, (LPOLESTR)L"Folders", 0);
		if (FAILED(hr)) {
			continue;
		}
		CComPtr<IDispatch> folders(result.pdispVal);

		int subFolderCount = GetSubFolderCount(folders);
		for (int i = subFolderCount; i >= 1; --i) {

			arg1 = CComVariant(i);
			hr = AutoWrap(DISPATCH_METHOD, &result, folders, (LPOLESTR)L"Item", 1, &arg1);
			if (FAILED(hr)) {
				continue;
			}

			CComPtr<IDispatch> subFolder(result.pdispVal);
			hr = AutoWrap(DISPATCH_METHOD, &result, subFolder, (LPOLESTR)L"DefaultItemType", 0);
			if (FAILED(hr) || result.intVal != 0) { 
				continue;
			}

			stk.push_front(STACK_ITEM{ parent, subFolder});
		}
	}

	return true;
}

// Outlookで表示中のフォルダを変更する
bool OutlookProxy::SelectFolder(CComPtr<IDispatch> folder)
{
	// 起動中のApplicaitonを取得
	CComPtr<IDispatch> app;
	if (in->GetRunningApp(app) == false) {
		return false;
	}

	CComVariant result;

	// Explorerオブジェクトを取得
	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, app, (LPOLESTR)L"ActiveExplorer", 0);
	if (FAILED(hr)) {
		spdlog::debug("Failed to get ActiveExplroer.\n");
		return false;
	}

	CComPtr<IDispatch> activeExplorer(result.pdispVal);
	if (!activeExplorer) {
		// ここは通常こないはず・・?
		return false;
	}

	// ウインドウを前面に出す
	HWND hwnd = FindWindow(_T("rctrl_renwnd32"), nullptr);
	if (IsWindow(hwnd)) {
		ScopeAttachThreadInput scope;

		// 最小化されていたら元に戻す
		LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
		if (style & WS_MINIMIZE) {
			PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		}

		SetForegroundWindow(hwnd);
	}


	// カレントフォルダを変更する
	CComVariant arg1((IDispatch*)folder);
	hr = AutoWrap(DISPATCH_METHOD, nullptr, activeExplorer, (LPOLESTR)L"CurrentFolder", 1, &arg1);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}
	


}}}
