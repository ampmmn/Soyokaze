#include "pch.h"
#include "MainWindowLayout.h"
#include "resource.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "mainwindow/layout/DefaultComponentPlacer.h"
#include "mainwindow/layout/WindowPosition.h"
#include "mainwindow/layout/MainWindowPlacement.h"


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

		if (mMainWnd) {
			HWND hwnd = mMainWnd->GetWindowObject()->GetSafeHwnd();
			mThisPtr->RecalcControls(hwnd, nullptr);

			struct LocalInputStatus : public LauncherInput {
				virtual bool HasKeyword() { return false; }
			} status;
			mThisPtr->UpdateInputStatus(&status, true); 
		}
	}

	void OnAppExit() override
	{
	}

	ComponentPlacer* CreateComponentPlacer()
	{
		if (mPlacer.get() == nullptr) {
			mPlacer.reset(CreateComponentPlacerIn());
		}
		return mPlacer.get();
	}

	ComponentPlacer* CreateComponentPlacerIn()
	{
		auto placement = new launcherapp::mainwindow::layout::MainWindowPlacement(mMainWnd);
		return new launcherapp::mainwindow::layout::DefaultComponentPlacer(placement);
	}

	LauncherMainWindowIF* mMainWnd{nullptr};
	MainWindowLayout* mThisPtr{nullptr};
	bool mIsMoveTemporary{false};
	CPoint mPositionToRestore{0,0};

	std::unique_ptr<ComponentPlacer> mPlacer;

	// ウインドウ位置を保存するためのクラス
	std::unique_ptr<WindowPosition> mWindowPositionPtr;

	bool mIsFirstUpdate{true};
	bool mIsPrevHasKeyword{false};
};

MainWindowLayout::MainWindowLayout(LauncherMainWindowIF* mainWnd) : in(new PImpl)
{
	in->mThisPtr = this;
	in->mMainWnd = mainWnd;
}

MainWindowLayout::~MainWindowLayout()
{
	// mWindowPositionPtrのインスタンス破棄時に位置情報を設定ファイルに保存する
}

// 入力状態が更新された
void MainWindowLayout::UpdateInputStatus(LauncherInput* status, bool isForceUpdate)
{
	ASSERT(status);

	// 初回 or 強制更新フラグ or 状態が変化した場合は実施するが、
	// そうでない場合はスキップする
	bool isCurHasKeyword = status->HasKeyword();
	bool isStatusUpdated = in->mIsPrevHasKeyword != isCurHasKeyword;
	if (isForceUpdate == false &&
	    in->mIsFirstUpdate  == false && 
			isStatusUpdated == false) {
		// スキップ
		return;
	}	

	in->mIsFirstUpdate = false;

	HWND mainWndHandle{in->mMainWnd->GetWindowObject()->GetSafeHwnd()};

	if (status->HasKeyword()) {
		// キーワードが入力されているため、候補欄を表示する
		in->mWindowPositionPtr->SyncPosition(mainWndHandle);
	}
	else {
		// キーワードは未入力であるため、候補欄を非表示にする
		CRect rc;
		GetWindowRect(mainWndHandle, &rc);
		RecalcWindowSize(mainWndHandle, status, WMSZ_TOP, rc); 
		in->mWindowPositionPtr->SetPositionTemporary(mainWndHandle, rc);
	}
	in->mIsPrevHasKeyword = isCurHasKeyword;
}


static bool IsValidForegroundWindow(HWND hwnd)
{
	if (IsWindow(hwnd) == FALSE) {
		return false;
	}

	TCHAR clsName[256];
	GetClassName(hwnd, clsName, 256);
	TCHAR caption[128];
	GetWindowText(hwnd, caption, 128);
	if (_tcscmp(clsName, _T("SysListView32")) == 0 && _tcscmp(caption, _T("FolderView")) == 0) {
		// デスクトップ上にアクティブなウインドウがない場合、ExcplorerのSysListView32が前面に来る
		SPDLOG_DEBUG("desktop");
		return false;
	}
	if (_tcscmp(clsName, _T("Shell_TrayWnd")) == 0 || _tcscmp(clsName, _T("Progman")) == 0) {
		// タスクトレイやプログラムマネージャのウインドウがとれることもある
		SPDLOG_DEBUG("traywnd or progman");
		return false;
	}

	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	if (style & WS_MINIMIZE) {
		// 最小化されている場合は対象外
		SPDLOG_DEBUG("minimized");
		return false;
	}
	if ((style & WS_VISIBLE) == 0) {
		// 非表示も対象外
		SPDLOG_DEBUG("invisible");
		return false;
	}

	return true;
}

// 指定した領域がモニターに収まっているかを判定する
static bool IsRectInMonitors(const CRect& rc)
{
	struct local_param {
		static BOOL CALLBACK EnumProc(HMONITOR hmon, HDC hdcMon, LPRECT lprcMon, LPARAM dwData) {
			UNREFERENCED_PARAMETER(lprcMon);
			UNREFERENCED_PARAMETER(hdcMon);

			auto thisPtr = (local_param*)dwData;

			MONITORINFO mi = { sizeof(MONITORINFO) };
			if (GetMonitorInfo(hmon, &mi)) {
				// モニター作業領域内に完全に収まっている領域の面積を得る
				auto validArea = thisPtr->mTargetRect & mi.rcWork;
				if (validArea.IsRectEmpty() == FALSE) {
					thisPtr->mValidArea += validArea.Width() * validArea.Height();
				}
			}

			return TRUE;
		}

		int mValidArea = 0;
		CRect mTargetRect;
	} param;

	param.mTargetRect = rc;
	EnumDisplayMonitors(nullptr, nullptr, local_param::EnumProc, (LPARAM)&param);

	// 画面内に半分収まってればヨシとする
	return param.mValidArea >= (rc.Width() * rc.Height()) / 2;
}


// ウインドウがアクティブになるときのウインドウ位置を決める
bool MainWindowLayout::RecalcWindowOnActivate(CWnd* wnd)
{
	spdlog::debug("MainWindowLayout::RecalcWindowOnActivate start");

	CPoint newPt{0,0};

	AppPreference* pref = AppPreference::Get();
	if (pref->IsShowMainWindowOnCurorPos()) {
		// マウスカーソル位置に入力欄ウインドウを表示する
		CPoint offset(-60, -50);
		POINT cursorPos;
		::GetCursorPos(&cursorPos);
		newPt.x = cursorPos.x + offset.x;
		newPt.y = cursorPos.y + offset.y;
		wnd->SetWindowPos(nullptr, newPt.x, newPt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		return true;
	}
	else if (pref->IsShowMainWindowOnActiveWindowCenter()) {
		// アクティブなウインドウの中央位置に入力欄ウインドウを表示する
		HWND fgWindow = ::GetForegroundWindow();
		if (IsValidForegroundWindow(fgWindow) == false) {
			return false;
		}

		// 基準とするウインドウのスクリーン座標上の領域
		CRect rc;
		::GetWindowRect(fgWindow, &rc);

		// 自ウインドウのスクリーン座標上の領域
		CRect rcSelf;
		wnd->GetWindowRect(&rcSelf);
		
		// 新しい位置を算出(基準ウインドウの中央)
		CSize offset(rcSelf.Width() / 2, rcSelf.Height() / 2);
		CRect newRect(rc.CenterPoint() - offset, rcSelf.Size()); 

		// 新しい位置がモニター内に収まっているかを判定する
		if (IsRectInMonitors(newRect) == false) {
			spdlog::warn(_T("out of monitor"));
			return false;
		}

		newPt.x = newRect.left;
		newPt.y = newRect.top;
		wnd->SetWindowPos(nullptr, newPt.x, newPt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		return true;
	}
	else {
		if (in->mIsMoveTemporary) {
			newPt = in->mPositionToRestore;
			wnd->SetWindowPos(nullptr, newPt.x, newPt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			in->mIsMoveTemporary = false;
		}
		return true;
	}
}

void MainWindowLayout::RestoreWindowPosition(CWnd* wnd, bool isForceReset)
{
	spdlog::debug("MainWindowLayout::RestoreWindowPosition");

	in->mWindowPositionPtr = std::make_unique<WindowPosition>();

	bool isSucceededToRestore = in->mWindowPositionPtr->Restore(wnd->GetSafeHwnd());
	if (isForceReset || isSucceededToRestore == false) {
		// 復元に失敗した場合は中央に表示
		wnd->SetWindowPos(nullptr, 0, 0, 600, 300, SWP_NOZORDER|SWP_NOMOVE);
		wnd->CenterWindow();
	}
}

/**
 	ウインドウのリサイズ中のサイズ制御をする
 	@param[in]     hwnd   入力画面のウインドウハンドル
 	@param[in]     status 入力状態
 	@param[in]     side   サイズ変更中の辺を表す値
 	@param[in,out] rect   現在の領域
*/
void MainWindowLayout::RecalcWindowSize(HWND hwnd, LauncherInput* status, UINT side, LPRECT rect)
{
	if (side == WMSZ_LEFT || side == WMSZ_RIGHT) {
		// 左右のリサイズは制約なし
		return ;
	}

	// ウインドウ領域とクライアント領域の差からフレーム(非クライアント領域)の高さを求める
	CRect rcC;
	GetClientRect(hwnd, rcC);
	CRect rcW;
	GetWindowRect(hwnd, rcW);
	int frameH = rcW.Height() - rcC.Height();

	// 触っているのが上側か?
	bool isUpside = side == WMSZ_TOP || side == WMSZ_TOPLEFT || side == WMSZ_TOPRIGHT;

	ComponentPlacer* placer = in->CreateComponentPlacer();

	// 最低限の高さ
	int minH = placer->GetMinimumHeight() + frameH;

	if (status && status->HasKeyword() == false) {
		// キーワード未入力状態の場合、サイズ変更自体を許さない

		if (isUpside) {
			rect->bottom = rect->top + minH;
		}
		else {
			rect->top = rect->bottom - minH;
		}
	}
	else {
		// キーワードが入力されている場合はサイズ変更を許すが、所定サイズより小さくさせない

		// ToDo: 最低限の候補欄の高さも加算
		int minCandidateH = placer->GetMinimumCandidateHeight();

		minH += minCandidateH;

		if (isUpside) {
			int allowedY = rect->bottom - minH;
			if (allowedY < rect->top) {
				rect->top = allowedY;
			}
		}
		else {
			int allowedY = rect->top + minH;
			if (rect->bottom < allowedY) {
				rect->bottom = allowedY;
			}
		}
	}
}

// ウインドウを非表示にする
void MainWindowLayout::HideWindow()
{
	CWnd* mainWnd = in->mMainWnd->GetWindowObject();
	if (mainWnd == nullptr) {
		return;
	}

	auto h = mainWnd->GetSafeHwnd();
	if (IsWindow(h)) {
		::ShowWindow(h, SW_HIDE);
	}
}

// ウインドウを一時的に移動する
bool MainWindowLayout::MoveTemporary(int vk)
{
	// 上下左右以外は無視
	if (vk != VK_UP && vk != VK_DOWN && vk != VK_LEFT && vk != VK_RIGHT) {
		return false;
	}

	HWND mainWndHandle{ in->mMainWnd->GetWindowObject()->GetSafeHwnd() };

	// モニタの枠を超えた移動をできないようにするため、モニタ座標を取得する
	HMONITOR mon = MonitorFromWindow(mainWndHandle, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = { sizeof(MONITORINFO) };
	GetMonitorInfo(mon, &mi);
	const CRect& rcMon = mi.rcWork;

	CRect rc;
	GetWindowRect(mainWndHandle, &rc);

	CPoint newPt = rc.TopLeft();

	// 移動量
	int offset = 200;

	// 向きに応じた移動後の座標値を決定する
	if (vk == VK_UP) {
		newPt.y -= offset;
		newPt.y = (std::max)(newPt.y, rcMon.top);
	}
	else if (vk == VK_DOWN) {
		newPt.y += offset;
		newPt.y = (std::min)(newPt.y, rcMon.bottom - rc.Height());
	}
	else if (vk == VK_LEFT) {
		newPt.x -= offset;
		newPt.x = (std::max)(newPt.x, rcMon.left);
	}
	else { // VK_RIGHT
		newPt.x += offset;
		newPt.x = (std::min)(newPt.x, rcMon.right - rc.Width());
	}
	
	// 移動
	SetWindowPos(mainWndHandle, nullptr, newPt.x, newPt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	// 次回は元の位置に戻すので、復元用の位置を覚えておく
	if (in->mIsMoveTemporary == false) {
		in->mPositionToRestore = rc.TopLeft();
		in->mIsMoveTemporary = true;
	}
	return true;
}

void MainWindowLayout::RecalcControls(HWND hwnd, LauncherInput* status)
{
	CWnd* mainWnd = in->mMainWnd->GetWindowObject();

	auto iconLabel = mainWnd->GetDlgItem(IDC_STATIC_ICON);
	if (iconLabel == nullptr) {
		// ウインドウ初期化が終わっていない場合はコントロールを取得できないのでここで抜ける
		return;
	}

	ComponentPlacer* placer = in->CreateComponentPlacer();

	// アイコン欄
	placer->PlaceIcon(iconLabel->GetSafeHwnd(), status);
	// 説明欄
	auto comment = mainWnd->GetDlgItem(IDC_STATIC_DESCRIPTION);
	placer->PlaceDescription(comment->GetSafeHwnd(), status);
	// ガイド欄
	auto guide = mainWnd->GetDlgItem(IDC_STATIC_GUIDE);
	placer->PlaceGuide(guide->GetSafeHwnd(), status);
	// 入力欄
	auto edit = mainWnd->GetDlgItem(IDC_EDIT_COMMAND);
	placer->PlaceEdit(edit->GetSafeHwnd(), status);
	// 候補欄
	auto listCtrl = mainWnd->GetDlgItem(IDC_LIST_CANDIDATE);
	placer->PlaceCandidateList(listCtrl->GetSafeHwnd(), status);
	// オプションボタン
	auto optionButton = mainWnd->GetDlgItem(IDC_BUTTON_OPTION);
	placer->PlaceOptionButton(optionButton->GetSafeHwnd(), status);

	placer->Apply(hwnd, status);


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
