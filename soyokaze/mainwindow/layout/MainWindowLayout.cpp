#include "pch.h"
#include "MainWindowLayout.h"
#include "resource.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "mainwindow/layout/DefaultComponentPlacer.h"
#include "mainwindow/layout/NoGuideComponentPlacer.h"
#include "mainwindow/layout/NoIconComponentPlacer.h"
#include "mainwindow/layout/NoGuideNoIconComponentPlacer.h"
#include "mainwindow/layout/WindowPosition.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace mainwindow {


using ComponentPlacer = launcherapp::mainwindow::layout::ComponentPlacer;

struct MainWindowLayout::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void OnAppFirstBoot() override
	{
	}
	void OnAppNormalBoot() override {}

	void OnAppPreferenceUpdated() override
	{
		mPlacer.reset();

		LoadSettings();

		if (mMainWnd) {
			mThisPtr->RecalcControls(mMainWnd, nullptr);
		}
	}

	void OnAppExit() override
	{
	}

	bool IsShowGuide()
	{
		return mIsShowGuide;
	}
	bool IsDrawIcon()
	{
		return mIsDrawIcon;
	}

	void LoadSettings()
	{
			AppPreference* pref = AppPreference::Get();
			mIsShowGuide = pref->IsShowGuide();
			mIsDrawIcon = pref->IsDrawIcon();

	}

	ComponentPlacer* CreateComponentPlacer(HWND hwnd)
	{
		if (mPlacer.get() == nullptr) {
			mPlacer.reset(CreateComponentPlacerIn(hwnd));
		}
		return mPlacer.get();
	}

	ComponentPlacer* CreateComponentPlacerIn(HWND hwnd)
	{
		if (mIsFirstCall) {
			LoadSettings();
			mIsFirstCall = false;
		}

		if (mIsShowGuide == false && mIsDrawIcon) {
			// ガイドなし、アイコンあり
			return new launcherapp::mainwindow::layout::NoGuideComponentPlacer(hwnd);
		}
		if (mIsShowGuide && mIsDrawIcon == false) {
			// ガイドあり、アイコンなし
			return new launcherapp::mainwindow::layout::NoIconComponentPlacer(hwnd);
		}
		if (mIsShowGuide == false && mIsDrawIcon == false) {
			// ガイドなし、アイコンなし
			return new launcherapp::mainwindow::layout::NoGuideNoIconComponentPlacer(hwnd);
		}
		else {
			return new launcherapp::mainwindow::layout::DefaultComponentPlacer(hwnd);
		}
	}

	HWND mMainWnd = nullptr;
	MainWindowLayout* mThisPtr = nullptr;
	bool mIsFirstCall = true;
	bool mIsShowGuide = false;
	bool mIsDrawIcon = true;

	std::unique_ptr<ComponentPlacer> mPlacer;

	// ウインドウ位置を保存するためのクラス
	std::unique_ptr<WindowPosition> mWindowPositionPtr;

	bool mIsFirstUpdate = true;
	bool mIsPrevHasKeyword = false;
};

MainWindowLayout::MainWindowLayout() : in(new PImpl)
{
	in->mThisPtr = this;
}

MainWindowLayout::~MainWindowLayout()
{
	// mWindowPositionPtrのインスタンス破棄時に位置情報を設定ファイルに保存する
}

// 入力状態が更新された
void MainWindowLayout::UpdateInputStatus(LauncherInput* status)
{
	ASSERT(status);

	bool isCurHasKeyword = status->HasKeyword();
	if (in->mIsFirstUpdate  == false && in->mIsPrevHasKeyword == isCurHasKeyword) {
		// 状態変化なし
		return;
	}	 
	in->mIsFirstUpdate = false;

	if (status->HasKeyword()) {
		// 候補欄を表示
		in->mWindowPositionPtr->SyncPosition(in->mMainWnd);
	}
	else {
		// 候補欄を非表示
		CRect rc;
		GetWindowRect(in->mMainWnd, &rc);
		RecalcWindowSize(in->mMainWnd, status, WMSZ_TOP, rc); 
		in->mWindowPositionPtr->SetPositionTemporary(in->mMainWnd, rc);
	}
	in->mIsPrevHasKeyword = isCurHasKeyword;
}

void MainWindowLayout::RestoreWindowPosition(CWnd* wnd, bool isForceReset)
{
	in->mWindowPositionPtr = std::make_unique<WindowPosition>();

	bool isSucceededToRestore = in->mWindowPositionPtr->Restore(wnd->GetSafeHwnd());
	if (isForceReset || isSucceededToRestore == false) {
		// 復元に失敗した場合は中央に表示
		wnd->SetWindowPos(nullptr, 0, 0, 600, 300, SWP_NOZORDER|SWP_NOMOVE);
		wnd->CenterWindow();
	}
}



void MainWindowLayout::OnShowWindow(CWnd* wnd, BOOL bShow, UINT nStatus)
{
	UNREFERENCED_PARAMETER(wnd);
	UNREFERENCED_PARAMETER(bShow);
	UNREFERENCED_PARAMETER(nStatus);
}

void MainWindowLayout::RecalcWindowSize(HWND hwnd, LauncherInput* status, UINT side, LPRECT rect)
{
	// 直近のウインドウを親ウインドウとして覚えておく(基本的に変化しないけど)
	in->mMainWnd = hwnd;


	if (side == WMSZ_LEFT || side == WMSZ_RIGHT) {
		// 左右のリサイズは制約なし
		return ;
	}

	CRect rcC;
	GetClientRect(hwnd, rcC);
	CRect rcW;
	GetWindowRect(hwnd, rcW);

	int frameH = rcW.Height() - rcC.Height();

	bool isUpside = side == WMSZ_TOP || side == WMSZ_TOPLEFT || side == WMSZ_TOPRIGHT;

	ComponentPlacer* placer = in->CreateComponentPlacer(hwnd);
	int minH = placer->GetMinimumHeight() + frameH;

	if (status && status->HasKeyword() == false) {
		// 上方向の場合
		if (isUpside) {
			rect->bottom = rect->top + minH;
		}
		// 下方向
		else {
			rect->top = rect->bottom - minH;
		}
	}
	else {
		// 上方向の場合
		if (isUpside) {
			int allowedY = rect->bottom - minH; 
			if (allowedY < rect->top) {
				rect->top = allowedY;
			}
		}
		// 下方向
		else {
			int allowedY = rect->top + minH; 
			if (rect->bottom < allowedY) {
				rect->bottom = allowedY;
			}
		}
	}
}

void MainWindowLayout::RecalcControls(HWND hwnd, LauncherInput* status)
{
	// 直近のウインドウを親ウインドウとして覚えておく(基本的に変化しないけど)
	in->mMainWnd = hwnd;

	CWnd* mainWnd = CWnd::FromHandle(hwnd);

	auto iconLabel = mainWnd->GetDlgItem(IDC_STATIC_ICON);
	if (iconLabel == nullptr) {
		// ウインドウ初期化が終わっていない場合はコントロールを取得できないのでここで抜ける
		return;
	}

	ComponentPlacer* placer = in->CreateComponentPlacer(hwnd);

	// アイコン欄
	placer->PlaceIcon(iconLabel->GetSafeHwnd());
	// 説明欄
	auto comment = mainWnd->GetDlgItem(IDC_STATIC_DESCRIPTION);
	placer->PlaceDescription(comment->GetSafeHwnd());
	// ガイド欄
	auto guide = mainWnd->GetDlgItem(IDC_STATIC_GUIDE);
	placer->PlaceGuide(guide->GetSafeHwnd());
	// 入力欄
	auto edit = mainWnd->GetDlgItem(IDC_EDIT_COMMAND);
	placer->PlaceEdit(edit->GetSafeHwnd());
	// 候補欄
	auto listCtrl = mainWnd->GetDlgItem(IDC_LIST_CANDIDATE);
	placer->PlaceCandidateList(listCtrl->GetSafeHwnd());

	placer->Apply(hwnd);


	// 何かしら入力がある状態であったら、位置情報を保持しておく
	if (status) {
		if (in->mWindowPositionPtr.get()) {
			if (status->HasKeyword()) {
				in->mWindowPositionPtr->Update(hwnd);
			}
			else {
				in->mWindowPositionPtr->UpdateExceptHeight(hwnd);
			}
		}
	}
}


}
}
