#include "GoToSlideProxyCommand.h"
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

REGISTER_PROXYCOMMAND(GoToSlideProxyCommand)

GoToSlideProxyCommand::GoToSlideProxyCommand()
{
}

GoToSlideProxyCommand::~GoToSlideProxyCommand()
{
}

std::string GoToSlideProxyCommand::GetName()
{
	return "gotoslide";
}

bool GoToSlideProxyCommand::Execute(json& json_req, json& json_res)
{
	int16_t pageIndex = json_req["pageIndex"].get<int16_t>();

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

	// アクティブなPresentationを取得
	DispWrapper window;
	app.GetPropertyObject(L"ActiveWindow", window);

	DispWrapper view;
	window.GetPropertyObject(L"View", view);

	view.CallVoidMethod(L"GoToSlide", pageIndex);

	json_res["result"] = true;
	return true;
}

} // end of namespace 



