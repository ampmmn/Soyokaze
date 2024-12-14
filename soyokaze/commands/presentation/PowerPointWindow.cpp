#include "pch.h"
#include "PowerPointWindow.h"
#include "utility/ScopeAttachThreadInput.h"
#include "commands/common/AutoWrap.h"
#include "icon/IconLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace presentation {

using DispWrapper = launcherapp::commands::common::DispWrapper;

struct PowerPointWindow::PImpl
{
	void GetView(DispWrapper& view)
	{
		// アクティブなPresentationを取得
		DispWrapper window;
		mApp.GetPropertyObject(L"ActiveWindow", window);

		window.GetPropertyObject(L"View", view);
	}

	DispWrapper mApp;
	HWND mHwnd = nullptr;
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

bool PowerPointWindow::GetAcitveWindow(std::unique_ptr<PowerPointWindow>& ptr)
{
	// PointPointのCLSIDを取得
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"PowerPoint.Application", &clsid);
	if (hr != S_OK) {
		// インストールされていない
		return false;
	}

	std::unique_ptr<PowerPointWindow> obj(new PowerPointWindow);

	// アプリケーションを取得
	CComPtr<IUnknown> unkPtr;
	hr = GetActiveObject(clsid, NULL, &unkPtr);
	if(FAILED(hr)) {
		return TRUE;
	}

	unkPtr->QueryInterface(&obj->in->mApp);

	CString caption = obj->in->mApp.GetPropertyString(L"Caption");
	if (caption.IsEmpty()) {
		return TRUE;
	}

	HWND hwnd = FindPresentaionWindowHwnd(caption);
	if (hwnd == nullptr) {
		return false;
	}
		
	obj->in->mHwnd = hwnd;
	ptr.reset(obj.release());

	return true;
}

bool PowerPointWindow::GoToSlide(int16_t pageIndex)
{
	// アクティブなPresentationを取得
	DispWrapper view;
	in->GetView(view);

	view.CallVoidMethod(L"GoToSlide", pageIndex);
	return true;
}

HWND PowerPointWindow::FindPresentaionWindowHwnd(const CString& caption)
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

			TCHAR caption[1024];
			GetWindowText(hwnd, caption, 1024);
			if (std::regex_match(caption, param->pat)) {
				param->hwndFound = hwnd;
				return FALSE;
			}
			return TRUE;
		}

		HWND hwndFound = nullptr;
		tregex pat;
	} findParam;

	// 指定したキャプションを持つウインドウハンドルを探す
	tstring patternStr(_T("^.*"));
	patternStr += (LPCTSTR)caption;
	patternStr += _T(".+$");

	findParam.pat = tregex(patternStr);

	EnumWindows(local_param::OnEnumWindows, (LPARAM)&findParam);

	return findParam.hwndFound;
}

HICON PowerPointWindow::ResolveIcon()
{
	static HICON sIcon = nullptr;
	if (sIcon) {
		return sIcon;
	}

	auto iconLoader = IconLoader::Get();

	// PointPointのCLSIDを取得
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"PowerPoint.Application", &clsid);
	if (hr != S_OK) {
		// インストールされていない
		return iconLoader->LoadUnknownIcon();
	}

	// アプリケーションを取得
	CComPtr<IUnknown> unkPtr;
	hr = GetActiveObject(clsid, NULL, &unkPtr);
	if(FAILED(hr)) {
		return iconLoader->LoadUnknownIcon();
	}

	DispWrapper app;
	unkPtr->QueryInterface(&app);

	CString caption = app.GetPropertyString(L"Caption");
	if (caption.IsEmpty()) {
		return iconLoader->LoadUnknownIcon();
	}

	HWND hwnd = FindPresentaionWindowHwnd(caption);
	if (IsWindow(hwnd) == FALSE) {
		return iconLoader->LoadUnknownIcon();
	}

	sIcon = iconLoader->LoadIconFromHwnd(hwnd);
	return sIcon;
}

} // end of namespace presentation
} // end of namespace commands
} // end of namespace launcherapp

