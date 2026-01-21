#include "pch.h"
#include "PowerPointWindow.h"
#include "utility/ScopeAttachThreadInput.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"
#include "icon/IconLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using json = nlohmann::json;

namespace launcherapp { namespace commands { namespace presentation {

using NormalPriviledgeProcessProxy = launcherapp::processproxy::NormalPriviledgeProcessProxy;

struct PowerPointWindow::PImpl
{
	HWND mHwnd{nullptr};
};


PowerPointWindow::PowerPointWindow() : in(new PImpl)
{
	in->mHwnd = nullptr;
}

PowerPointWindow::~PowerPointWindow()
{
}

bool PowerPointWindow::Activate(bool isShowMaximize)
{
	HWND hwnd = in->mHwnd;

	if (IsWindow(hwnd) == FALSE) {
		return false;
	}

	ScopeAttachThreadInput scope;

	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	if (isShowMaximize && (style & WS_MAXIMIZE) == 0) {
			// Ctrlキーが押されていたら最大化表示する
			PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}
	else if (style & WS_MINIMIZE) {
		// 最小化されていたら元に戻す
		PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
	}

	SetForegroundWindow(hwnd);

	return true;
}

// アクティブなPowerpointのウインドウを取得する
static bool GetActivePowerPointWindow(HWND& hwnd)
{
	std::string dst;

	try {
		json json_req;
		json_req["command"] = "getactivepointerpointwindow";

		auto proxy = NormalPriviledgeProcessProxy::GetInstance();

		// リクエストを送信する
		json json_res;
		if (proxy->SendRequest(json_req, json_res) == false) {
			return false;
		}
		
		if (json_res["result"] == false) {
			return false;
		}

		hwnd = (HWND)json_res["hwnd"].get<uint64_t>();
		return true;
	}
	catch (const json::exception& e) {
		spdlog::error("[GetActivePowerPointWindow] Unexpected exception occurred. {}", e.what());
	}
	return false;
}


bool PowerPointWindow::GetAcitveWindow(std::unique_ptr<PowerPointWindow>& ptr)
{
	// PowerPointアプリのウインドウハンドルを取得する
	HWND hwnd = nullptr;
	if (GetActivePowerPointWindow(hwnd) == false || IsWindow(hwnd) == FALSE) {
		return false;
	}

	std::unique_ptr<PowerPointWindow> obj(new PowerPointWindow);
	obj->in->mHwnd = hwnd;
	ptr.reset(obj.release());

	return true;
}

bool PowerPointWindow::GoToSlide(int16_t pageIndex)
{
	try {
		json json_req;
		json_req["command"] = "gotoslide";
		json_req["pageIndex"] = pageIndex;

		auto proxy = NormalPriviledgeProcessProxy::GetInstance();

		// リクエストを送信する
		json json_res;
		if (proxy->SendRequest(json_req, json_res) == false) {
			return false;
		}
		
		return json_res["result"];
	}
	catch (const json::exception& e) {
		spdlog::error("[PowerPointWindow::GoToSlide] Unexpected exception occurred. {}", e.what());
	}
	return false;
}

}}} // end of namespace launcherapp::commands::presentation
