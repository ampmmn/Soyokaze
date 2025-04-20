#include "GetActivePointPointWindowProxyCommand.h"
#include <windows.h>
#include "StringUtil.h"
#include "AutoWrap.h"
#include <oleauto.h>
#include <atlbase.h>
#include <regex>
#include <spdlog/spdlog.h>
#include <fmt/core.h>

using json = nlohmann::json;

namespace launcherproxy { 

REGISTER_PROXYCOMMAND(GetActivePointPointWindowProxyCommand)

GetActivePointPointWindowProxyCommand::GetActivePointPointWindowProxyCommand()
{
}

GetActivePointPointWindowProxyCommand::~GetActivePointPointWindowProxyCommand()
{
}

std::string GetActivePointPointWindowProxyCommand::GetName()
{
	return "getactivepointerpointwindow";
}

static HWND FindPresentaionWindowHwnd(const std::wstring& caption)
{
	struct local_param {
		static BOOL CALLBACK OnEnumWindows(HWND hwnd, LPARAM lParam) {

			LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
			LONG_PTR styleRequired = (WS_VISIBLE);
			if ((style & styleRequired) != styleRequired) {
				// 非表示のウインドウと、タイトルを持たないウインドウは対象外
				return TRUE;
			}
			auto param = (local_param*)lParam;

			wchar_t caption[1024];
			GetWindowTextW(hwnd, caption, 1024);
			if (std::regex_match(caption, param->pat)) {
				param->hwndFound = hwnd;
				return FALSE;
			}
			return TRUE;
		}

		HWND hwndFound = nullptr;
		std::wregex pat;
	} findParam;

	// 指定したキャプションを持つウインドウハンドルを探す
	std::wstring patternStr(L"^.*");
	patternStr += caption;
	patternStr += L".+$";
	findParam.pat = std::wregex(patternStr);

	EnumWindows(local_param::OnEnumWindows, (LPARAM)&findParam);

	return findParam.hwndFound;
}

bool GetActivePointPointWindowProxyCommand::Execute(json& json_req, json& json_res)
{
	// PointPointのCLSIDを取得
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"PowerPoint.Application", &clsid);
	if (hr != S_OK) {
		// インストールされていない
		json_res["result"] = false;
		json_res["reason"] = "PowerPoint is not installed.";
		return true;
	}

	// アプリケーションを取得
	CComPtr<IUnknown> unkPtr;
	hr = GetActiveObject(clsid, NULL, &unkPtr);
	if(FAILED(hr)) {
		// 起動されていない
		json_res["result"] = false;
		json_res["reason"] = "PowerPoint is not run.";
		return true;
	}

	DispWrapper app;
	unkPtr->QueryInterface(&app);

	std::wstring caption = app.GetPropertyString(L"Caption");
	if (caption.empty()) {
		json_res["result"] = false;
		json_res["reason"] = "Failed to get caption.";
		return true;
	}

	HWND hwnd = FindPresentaionWindowHwnd(caption);
	if (IsWindow(hwnd) == FALSE) {
		json_res["result"] = false;
		json_res["reason"] = "Failed to get window.";
		return true;
	}

	json_res["hwnd"] = (uint64_t)hwnd;
	json_res["result"] = true;
	return true;
}

} // end of namespace 



