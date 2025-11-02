#include "OutlookSelectFolderProxyCommand.h"
#include <windows.h>
#include "StringUtil.h"
#include "ScopeAttachThreadInput.h"
#include "AutoWrap.h"
#include <atlbase.h>
#include <deque>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace launcherproxy {


struct OutlookSelectFolderProxyCommand::PImpl
{
	bool GetRunningApp(CComPtr<IDispatch>& app);
};

// 起動中のOutlook.Applicationインスタンスを取得する
bool OutlookSelectFolderProxyCommand::PImpl::GetRunningApp(CComPtr<IDispatch>& app)
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



REGISTER_PROXYCOMMAND(OutlookSelectFolderProxyCommand)

OutlookSelectFolderProxyCommand::OutlookSelectFolderProxyCommand() : in(new PImpl)
{
}

OutlookSelectFolderProxyCommand::~OutlookSelectFolderProxyCommand()
{
}

std::string OutlookSelectFolderProxyCommand::GetName()
{
	return "outlook_selectfolder";
}

bool OutlookSelectFolderProxyCommand::Execute(json& json_req, json& json_res)
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
		json_res["reason"] = "Failed to get ActiveExplroer.\n";
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


	// EntryIDからフォルダを得る
	CComVariant arg1(L"MAPI");
	hr = AutoWrap(DISPATCH_METHOD, &result, app, (LPOLESTR)L"GetNameSpace", 1, &arg1);
	if (FAILED(hr)) {
		json_res["reason"] = "Failed to GetNameSpace.";
		return false;
	}
	CComPtr<IDispatch> mapi(result.pdispVal);

	std::wstring entry_id;
	utf2utf(json_req["entry_id"], entry_id);
	arg1 = CComVariant(entry_id.c_str());

	hr = AutoWrap(DISPATCH_METHOD, &result, mapi, (LPOLESTR)L"GetFolderFromID", 1, &arg1);
	if (FAILED(hr)) {
		json_res["reason"] = "Failed to call GetFolderFromID.";
		return false;
	}

	// カレントフォルダを変更する
	arg1 = CComVariant((IDispatch*)result.pdispVal);
	hr = AutoWrap(DISPATCH_METHOD, nullptr, activeExplorer, (LPOLESTR)L"CurrentFolder", 1, &arg1);
	if (FAILED(hr)) {
		return false;
	}

	json_res["result"] = true;
	return true;
}

}



