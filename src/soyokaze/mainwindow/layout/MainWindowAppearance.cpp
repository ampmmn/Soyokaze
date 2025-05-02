#include "pch.h"
#include "MainWindowAppearance.h"
#include "setting/AppPreference.h"
#include "gui/ColorSettings.h"
#include "icon/IconLabel.h"
#include "mainwindow/layout/WindowTransparency.h"
#include "utility/Accessibility.h"

#if _MSC_VER < 1920
#define DWMWA_COLOR_DEFAULT  0xFFFFFFFF
#define DWMWA_BORDER_COLOR   34
#define DWMWA_CAPTION_COLOR  35
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace mainwindow {

struct MainWindowAppearance::PImpl
{
	// フォントを更新
	bool UpdateFont()
	{
		auto pref = AppPreference::Get();

		// アプリ設定上のフォント名を取得
		CString fontName;
		if (pref->GetMainWindowFontName(fontName) == false) {
			return false;
		}

		CWnd* mainWnd = mMainWnd->GetWindowObject();
		// ウインドウにセットされた現在のフォント情報を取得
		LOGFONT lf;
		mainWnd->GetFont()->GetLogFont(&lf);

		// フォント情報をベースに、アプリ設定上のフォント名のみ変えて、フォントを作り直す
		_tcsncpy_s(lf.lfFaceName, LF_FACESIZE, fontName, _TRUNCATE);
		if (mFont.m_hObject) {
			mFont.DeleteObject();
		}

		// サイズ計算(ポイントサイズからlfHeight)
		HDC hdc = GetDC(nullptr);
		int pointSize = pref->GetMainWindowFontSize();
		lf.lfHeight = -MulDiv(pointSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		ReleaseDC(nullptr, hdc);

		mFont.CreateFontIndirectW(&lf);

		// 作りなおしたフォントをウインドウや子ウインドウにセットする
		mainWnd->SetFont(&mFont);
		mainWnd->SendMessageToDescendants(WM_SETFONT, (WPARAM)mFont.m_hObject, MAKELONG(FALSE, 0), FALSE);

		return true;
	}

	LauncherMainWindowIF* mMainWnd{nullptr};
	// 色設定をリセットするか?
	bool mShouldColorInit{true};
	// フォント
	CFont mFont;
	// ウインドウの透明度を制御するためのクラス
	std::unique_ptr<WindowTransparency> mWindowTransparencyPtr;

	bool mIsBlockDeactivateOnUnfocus{ false};
};

MainWindowAppearance::MainWindowAppearance(LauncherMainWindowIF* mainWnd) : in(new PImpl)
{
	in->mMainWnd = mainWnd;

	auto pref = AppPreference::Get();

	// リスナー登録
	pref->RegisterListener(this);

	// 
	in->UpdateFont();
}

MainWindowAppearance::~MainWindowAppearance()
{
	AppPreference::Get()->UnregisterListener(this);
}

CFont* MainWindowAppearance::GetFont()
{
	return &in->mFont;
}

void MainWindowAppearance::OnShowWindow(BOOL bShow, UINT nStatus)
{
	UNREFERENCED_PARAMETER(nStatus);

	if (bShow == FALSE) {
		// 表示になるときは何もしない
		return;
	}

	// 表示するタイミングで、前回と色設定が変わっている場合は色設定をウインドウに反映する
	// 非クライアント領域の色を変える
	if (in->mShouldColorInit == false) {
		return;
	}

	// 色設定を取得
	auto colorSettings = ColorSettings::Get();
	auto colorScheme = colorSettings->GetCurrentScheme();

	// システム設定を使うかどうかに応じて色を変える
	bool isDefaultColor = colorSettings->IsSystemSettings();
	COLORREF cr = DWMWA_COLOR_DEFAULT;
	if (isDefaultColor == false) {
		cr = colorScheme->GetBackgroundColor();
	}

	// ボーダー領域(非クライアント)の色を設定する
	// (Windows11より以前の環境では設定できない。APIをコールしても失敗する)
	HWND wndHandle = in->mMainWnd->GetWindowObject()->GetSafeHwnd();
	::DwmSetWindowAttribute(wndHandle, DWMWA_BORDER_COLOR, &cr, sizeof(cr));
	::DwmSetWindowAttribute(wndHandle, DWMWA_CAPTION_COLOR, &cr, sizeof(cr));

	auto iconLabel = in->mMainWnd->GetIconLabel();
	iconLabel->SetBackgroundColor(isDefaultColor, colorScheme->GetBackgroundColor());

	in->mShouldColorInit = false;
}

void MainWindowAppearance::OnActivate(UINT nState, CWnd* wnd, BOOL bMinimized)
{
	UNREFERENCED_PARAMETER(wnd);
	UNREFERENCED_PARAMETER(bMinimized);

	// メインウインドウがフォーカスを失っても非表示にしない設定であれば透過状態を更新
	if (in->mIsBlockDeactivateOnUnfocus == false) {

		// 透明度制御
		if (in->mWindowTransparencyPtr.get() == nullptr) {
			HWND hwnd = in->mMainWnd->GetWindowObject()->GetSafeHwnd();
			in->mWindowTransparencyPtr = std::make_unique<WindowTransparency>();
			in->mWindowTransparencyPtr->SetWindowHandle(hwnd);
		}

		in->mWindowTransparencyPtr->UpdateActiveState(nState);
	}
}

HBRUSH MainWindowAppearance::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor, HBRUSH defBr)
{
	UNREFERENCED_PARAMETER(pWnd);

	// ハイコントラストモードの場合はシステム設定をそのまま使う
	if (::utility::IsHighContrastMode()) {
		return defBr;
	}

	auto colorSettings = ColorSettings::Get();
	auto colorScheme = colorSettings->GetCurrentScheme();

	if (nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC) {
		pDC->SetTextColor(colorScheme->GetTextColor());
		pDC->SetBkColor(colorScheme->GetBackgroundColor());
		return colorScheme->GetBackgroundBrush();
	}
	else if (nCtlColor == CTLCOLOR_EDIT) {
		pDC->SetTextColor(colorScheme->GetEditTextColor());
		pDC->SetBkColor(colorScheme->GetEditBackgroundColor());
		return colorScheme->GetEditBackgroundBrush();
	}

	return defBr;
}

// メインウインドウがフォーカスを失っても非表示にしないようにする
void MainWindowAppearance::SetBlockDeactivateOnUnfocus(bool isBlock)
{
	in->mIsBlockDeactivateOnUnfocus = isBlock;
}

void MainWindowAppearance::OnAppFirstBoot()
{
}

void MainWindowAppearance::OnAppNormalBoot()
{
}


void MainWindowAppearance::OnAppPreferenceUpdated()
{
	in->UpdateFont();
	// 色を再設定する
	in->mShouldColorInit = true;
}

void MainWindowAppearance::OnAppExit()
{
}



}
}



