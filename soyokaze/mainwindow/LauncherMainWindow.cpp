
// LauncherMainWindow.cpp : 実装ファイル
//

#include "pch.h"
#include "framework.h"
#include "app/LauncherApp.h"
#include "app/Manual.h"
#include "mainwindow/LauncherMainWindow.h"
#include "mainwindow/CandidateListCtrl.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "afxdialogex.h"
#include "utility/WindowPosition.h"
#include "SharedHwnd.h"
#include "hotkey/AppHotKey.h"
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/KeyInputWatch.h"
#include "mainwindow/WindowTransparency.h"
#include "icon/IconLoader.h"
#include "setting/AppPreference.h"
#include "mainwindow/AppSound.h"
#include "mainwindow/LauncherWindowEventDispatcher.h"
#include "utility/ProcessPath.h"
#include "utility/ScopeAttachThreadInput.h"
#include "mainwindow/MainWindowHotKey.h"
#include "mainwindow/OperationWatcher.h"
#include "macros/core/MacroRepository.h"
#include "CandidateList.h"
#include <algorithm>
#include <thread>
#include <wtsapi32.h>

#pragma comment(lib, "Wtsapi32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp;

// Ctrl-Altキーで半透明にする隠しコマンド用のタイマー
constexpr UINT TIMERID_KEYSTATE = 1;
// ランチャーのタイマーイベントをリスナーに通知する用のタイマー
constexpr UINT TIMERID_OPERATION = 2;

struct LauncherMainWindow::PImpl
{
	PImpl(LauncherMainWindow* thisPtr) : 
		mDropTargetDialog(thisPtr),
		mDropTargetEdit(thisPtr)
	{
	}

	void RestoreWindowPosition(CWnd* thisPtr, bool isForceReset);
	void UpdateCommandString(core::Command* cmd, int& startPos, int& endPos);

	void EnumTokenPos(std::vector<int>& tokenPos);
	bool GetTrailingString(CString& text);

	// キーワード入力欄の文字列
	CString mCommandStr;
	// 最後に外部からの入力によって更新された時点での文字列
	CString mLastInputStr;
	CString mGuideStr;
	// 現在選択中のコマンドの説明
	CString mDescriptionStr;

	// 現在の候補一覧(選択中の項目もここが管理する)
	CandidateList mCandidates;
	// 候補一覧表示用リストボックス
	CandidateListCtrl mCandidateListBox;

	// ウインドウハンドル(共有メモリに保存する用)
	std::unique_ptr<SharedHwnd> mSharedHwnd;
	   // 後で起動したプロセスから有効化するために共有メモリに保存している

	// キーワード入力エディットボックス
	KeywordEdit mKeywordEdit;
	//DWORD mLastCaretPos;

	// キー入力監視(修飾キー入力によるホットキー機能用)
	KeyInputWatch mKeyInputWatch;

	// 外部からのコマンド受付用エディットボックス
	CmdReceiveEdit mCmdReceiveEdit;

	// アイコン描画用ラベル
	CaptureIconLabel mIconLabel;

	// 入力画面を呼び出すホットキー関連の処理をする
	std::unique_ptr<AppHotKey> mHotKeyPtr;
	std::unique_ptr<MainWindowHotKey> mMainWindowHotKeyPtr;

	// ウインドウ位置を保存するためのクラス
	std::unique_ptr<WindowPosition> mWindowPositionPtr;

	// ウインドウの透明度を制御するためのクラス
	std::unique_ptr<WindowTransparency> mWindowTransparencyPtr;
	//
	bool mIsPrevTransparentState = false;
	bool mIsQueryDoing = false;
	bool mIsBlockDeactivateOnUnfocus = false;

	CFont mFontDlg;

	// ドロップターゲット
	LauncherDropTarget mDropTargetDialog;
	LauncherDropTarget mDropTargetEdit;

	// 稼働状況を監視する(長時間連続稼働を警告する目的)
	OperationWatcher mOpWatcher;

	//
	UINT_PTR mInputTimerId = 0;

};

void LauncherMainWindow::PImpl::RestoreWindowPosition(CWnd* thisPtr, bool isForceReset)
{
	mWindowPositionPtr = std::make_unique<WindowPosition>();

	bool isSucceededToRestore = mWindowPositionPtr->Restore(thisPtr->GetSafeHwnd());
	if (isForceReset || isSucceededToRestore == false) {
		// 復元に失敗した場合は中央に表示
		thisPtr->SetWindowPos(nullptr, 0, 0, 600, 300, SWP_NOZORDER|SWP_NOMOVE);
		thisPtr->CenterWindow();
	}
}

void LauncherMainWindow::PImpl::UpdateCommandString(core::Command* cmd, int& startPos, int& endPos)
{
	auto cmdName = cmd->GetName();

	if (cmdName.Find(mLastInputStr) == 0 && cmdName.GetLength() > mLastInputStr.GetLength()) {
		startPos = mLastInputStr.GetLength();
		endPos = cmdName.GetLength();
		mCommandStr = cmdName;
	}
	else {
		mCommandStr = mLastInputStr;
		startPos = mCommandStr.GetLength();
		endPos = mCommandStr.GetLength();
	}

	// 説明の更新
	mDescriptionStr = cmd->GetDescription();
	if (mDescriptionStr.IsEmpty()) {
		mDescriptionStr = cmd->GetName();
	}

	// ガイド文字列の更新
	mGuideStr = cmd->GetGuideString();

	// アイコン更新
	mIconLabel.DrawIcon(cmd->GetIcon());
}

void LauncherMainWindow::PImpl::EnumTokenPos(std::vector<int>& tokenPos)
{
	std::vector<int> tmpPos;

	bool inToken = false;
	bool inQuate = false;
	int len = mCommandStr.GetLength();
	for (int i = 0; i < len; ++i) {
		auto c = mCommandStr[i];
		if (inQuate == false && c == _T('"')) {
		 	inQuate = true;
			inToken = true;
			tmpPos.push_back(i);
			continue;
	 	}
		if (inQuate && c == _T('"')) {
		 	inQuate = false;
			continue;
	 	}
		if (inQuate == false && c == _T(' ')) {
			inToken = false;
			continue;
		}
		else {
			if (inToken) {
				continue;
			}
			tmpPos.push_back(i);
			inToken = true;
		}
	}

	tokenPos.swap(tmpPos);
}

// 現在のキャレット位置より後ろにある文字を取得する
bool LauncherMainWindow::PImpl::GetTrailingString(CString& text)
{
	std::vector<int> tokenPos;
	EnumTokenPos(tokenPos);

	int startPos;
	int endPos;
	mKeywordEdit.GetSel(startPos, endPos);
	SPDLOG_DEBUG(_T("endPos:{}"), endPos);

	auto it = tokenPos.begin();
	for (; it != tokenPos.end(); ++it) {
		auto pos = *it;
		if (endPos < pos) {
			break;
		}
	}
	if (it == tokenPos.end()) {
		return false;
	}

	text = mCommandStr.Mid(*it);
	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


// LauncherMainWindow ダイアログ

LauncherMainWindow::LauncherMainWindow(CWnd* pParent /*=nullptr*/)
	: CDialogEx(AppPreference::Get()->IsShowGuide() ? IDD_MAIN_GUIDE : IDD_MAIN, pParent),
	in(std::make_unique<PImpl>(this))
{
	in->mWindowTransparencyPtr = std::make_unique<WindowTransparency>();

	in->mCandidateListBox.SetCandidateList(&in->mCandidates);
	AppPreference::Get()->RegisterListener(this);
}

LauncherMainWindow::~LauncherMainWindow()
{
	in->mCandidates.RemoveListener(&in->mCandidateListBox);
	AppPreference::Get()->UnregisterListener(this);

	// mWindowPositionPtrのインスタンス破棄時に位置情報を設定ファイルに保存する
}

void LauncherMainWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COMMAND, in->mCommandStr);
	DDX_Text(pDX, IDC_STATIC_GUIDE, in->mGuideStr);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, in->mDescriptionStr);
}

BEGIN_MESSAGE_MAP(LauncherMainWindow, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDIT_COMMAND, OnEditCommandChanged)
	ON_WM_SHOWWINDOW()
	ON_WM_NCHITTEST()
	ON_WM_ACTIVATE()
	ON_MESSAGE(WM_APP+1, OnKeywordEditNotify)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_CANDIDATE, OnLvnItemChange)
	ON_NOTIFY(NM_CLICK, IDC_LIST_CANDIDATE, OnNMClick)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CANDIDATE, OnNMDblclk)
	ON_WM_SIZE()
	ON_MESSAGE(WM_APP+2, OnUserMessageActiveWindow)
	ON_MESSAGE(WM_APP+3, OnUserMessageRunCommand)
	ON_MESSAGE(WM_APP+4, OnUserMessageDragOverObject)
	ON_MESSAGE(WM_APP+5, OnUserMessageDropObject)
	ON_MESSAGE(WM_APP+6, OnUserMessageCaptureWindow)
	ON_MESSAGE(WM_APP+7, OnUserMessageHideAtFirst)
	ON_MESSAGE(WM_APP+8, OnUserMessageAppQuit)
	ON_MESSAGE(WM_APP+9, OnUserMessageSetClipboardString)
	ON_MESSAGE(WM_APP+10, OnUserMessageGetClipboardString)
	ON_MESSAGE(WM_APP+11, OnUserMessageSetText)
	ON_MESSAGE(WM_APP+12, OnUserMessageSetSel)
	ON_MESSAGE(WM_APP+13, OnUserMessageQueryComplete)
	ON_MESSAGE(WM_APP+14, OnUserMessageBlockDeactivateOnUnfocus)
	ON_MESSAGE(WM_APP+15, OnUserMessageUpdateCandidate)
	ON_WM_CONTEXTMENU()
	ON_WM_ENDSESSION()
	ON_WM_TIMER()
	ON_COMMAND(ID_HELP, OnCommandHelp)
	ON_MESSAGE(WM_WTSSESSION_CHANGE, OnMessageSessionChange)
	ON_COMMAND_RANGE(core::CommandHotKeyManager::ID_LOCAL_START, 
	                 core::CommandHotKeyManager::ID_LOCAL_END, OnCommandHotKey)
END_MESSAGE_MAP()

void LauncherMainWindow::ActivateWindow(HWND hwnd)
{
	::PostMessage(hwnd, WM_APP+2, 0, 0);
}

void LauncherMainWindow::ActivateWindow()
{
	if (IsWindow(GetSafeHwnd())) {
		LauncherMainWindow::ActivateWindow(GetSafeHwnd());
	}
}

void LauncherMainWindow::HideWindow()
{
	GetCommandRepository()->Unactivate();
	::ShowWindow(GetSafeHwnd(), SW_HIDE);
}


void LauncherMainWindow::ShowHelpTop()
{
	SPDLOG_DEBUG(_T("start"));
	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate(_T("Top"));
}

void LauncherMainWindow::ShowHelpInputWindow()
{
	SPDLOG_DEBUG(_T("start"));
	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate(_T("InputWindow"));
}


/**
 * ActiveWindow経由の処理
 * (後続プロセスから処理できるようにするためウインドウメッセージ経由で処理している)
 */
LRESULT LauncherMainWindow::OnUserMessageActiveWindow(WPARAM wParam, LPARAM lParam)
{
	bool isShowForce = ((wParam & 0x1) != 0);

	HWND hwnd = GetSafeHwnd();
	if (isShowForce || ::IsWindowVisible(hwnd) == FALSE) {

		AppPreference* pref= AppPreference::Get();

		// 非表示状態なら表示
		ScopeAttachThreadInput scope;

		if (pref->IsShowMainWindowOnCurorPos()) {
			// マウスカーソル位置に入力欄ウインドウを表示する
			CPoint offset(-60, -50);
			POINT cursorPos;
			::GetCursorPos(&cursorPos);
			::SetWindowPos(hwnd, nullptr, cursorPos.x + offset.x, cursorPos.y + offset.y, 0, 0, 
			               SWP_NOZORDER | SWP_NOSIZE);
		}

		::ShowWindow(hwnd, SW_SHOW);
		::SetForegroundWindow(hwnd);
		::BringWindowToTop(hwnd);

		if (pref->IsIMEOffOnActive()) {
			in->mKeywordEdit.SetIMEOff();
		}

		GetCommandRepository()->Activate();
	}
	else {
		// 表示状態ではあるが、非アクティブならアクティブにする
		if (hwnd != ::GetActiveWindow()) {
			ScopeAttachThreadInput scope;
			::ShowWindow(hwnd, SW_SHOW);
			::SetForegroundWindow(hwnd);
			::BringWindowToTop(hwnd);
			return 0;
		}

		// 表示状態の場合はアプリ設定に応じて動作を変える

		AppPreference* pref= AppPreference::Get();
		if (pref->IsShowToggle()) {
			// トグル表示設定にしている場合は非表示にする
			HideWindow();
		}
	}
	return 0;
}

/**
 * 後続プロセスから "-c <文字列>" 経由でコマンド実行指示を受け取ったときの処理
 */
LRESULT LauncherMainWindow::OnUserMessageRunCommand(WPARAM wParam, LPARAM lParam)
{
	SPDLOG_DEBUG(_T("start"));

	LPCTSTR text = (LPCTSTR)lParam;
	if (text == nullptr) {
		SPDLOG_WARN(_T("text is null"));
		return 0;
	}

	bool isPasteOnly = (wParam == 1);

	if(isPasteOnly) {
		ExecuteCommand(text);
	}
	else {
		ExecuteCommand(text);
	}
	return 0;
}

/**
 * 入力欄にテキストをセットする処理
 */
LRESULT LauncherMainWindow::OnUserMessageSetText(WPARAM wParam, LPARAM lParam)
{
	SPDLOG_DEBUG(_T("start"));

	LPCTSTR text = (LPCTSTR)lParam;
	if (text == nullptr) {
		SPDLOG_WARN(_T("text is null"));
		return 0;
	}
	SPDLOG_DEBUG(_T("text:{}"), text);

	in->mCommandStr = text;
	in->mLastInputStr = text;
	in->mKeywordEdit.SetWindowText(text);
	in->mKeywordEdit.SetCaretToEnd();

	QuerySync();

	return 0;
}

/**
 * 入力欄の選択範囲を設定する
 */
LRESULT LauncherMainWindow::OnUserMessageSetSel(WPARAM wParam, LPARAM lParam)
{
	SPDLOG_DEBUG(_T("args wp:{0} lp:{1}"), wParam, lParam);

	int startChar = (int)wParam;

	int endChar = startChar;
	int lp = (int)lParam;
	if(startChar == -1) {
		// startChar=-1は選択解除として扱う
	}
	else if (lp >= 0) {
		endChar = startChar + lp;
	}
	else {
		// lParamが負の値の場合、逆方向からの選択として扱う(bluewindの挙動に合わせる)
		CString str;
		in->mKeywordEdit.GetWindowText(str);

		if (startChar == 0) {
			startChar = str.GetLength();
		}

		startChar = startChar + lp;
		endChar = startChar - lp;
		SPDLOG_DEBUG(_T("text:{0} length:{1} startChar:{2} endChar:{3}"), 
		             (LPCTSTR)str, str.GetLength(), startChar, endChar);
	}

	in->mKeywordEdit.SetSel(startChar, endChar);
	return 0;
}

LRESULT LauncherMainWindow::OnUserMessageQueryComplete(WPARAM wParam, LPARAM lParam)
{
	in->mIsQueryDoing = false;
	auto commands = (std::vector<launcherapp::core::Command*>*)lParam;
	if (commands != nullptr) {
		in->mCandidates.SetItems(*commands);
		delete commands;
	}
	else {
		in->mCandidates.Clear();
	}

	UpdateCandidates();

	return 0;
}

LRESULT LauncherMainWindow::OnUserMessageBlockDeactivateOnUnfocus(WPARAM wParam, LPARAM lParam)
{
	// メインウインドウがフォーカスを失っても非表示にしないようにする
	in->mIsBlockDeactivateOnUnfocus = (lParam != 0);
	return 0;
}

LRESULT LauncherMainWindow::OnUserMessageUpdateCandidate(WPARAM wParam, LPARAM lParam)
{
	if (::IsWindowVisible(GetSafeHwnd()) == FALSE) {
		// ウインドウを表示していない場合は更新しない
		return 0;
	}

	// 候補欄を更新するため、再度検索リクエストを出す
	QueryAsync();
	return 0;
}


LRESULT 
LauncherMainWindow::OnUserMessageDragOverObject(
	WPARAM wParam,
 	LPARAM lParam
)
{
	SPDLOG_DEBUG(_T("start"));

	CWnd* wnd = (CWnd*)lParam;
	if (wnd == this) {
		SetDescription(CString((LPCTSTR)IDS_NEWREGISTER));
	}
	else if (wnd == &in->mKeywordEdit) {
		SetDescription(CString((LPCTSTR)IDS_PASTE));
	}
	return 0;
}

LRESULT 
LauncherMainWindow::OnUserMessageDropObject(
	WPARAM wParam,
 	LPARAM lParam
)
{
	SPDLOG_DEBUG(_T("start"));

	COleDataObject* dataObj = (COleDataObject*)wParam;
	CWnd* wnd = (CWnd*)lParam;

	if (dataObj->IsDataAvailable(CF_HDROP)) {
		std::vector<CString> files;

		STGMEDIUM st;
		if (dataObj->GetData(CF_HDROP, &st) ) {
			HDROP dropInfo = static_cast<HDROP>(st.hGlobal);

			int fileCount = (int)DragQueryFile( dropInfo, (UINT)-1, NULL, 0 );
			files.reserve(fileCount);

			TCHAR filePath[MAX_PATH_NTFS];
			for (int i = 0; i < fileCount; ++i) {
				DragQueryFile(dropInfo, i, filePath, MAX_PATH_NTFS);
				files.push_back(filePath);
			}
		}

		ASSERT(files.size() > 0);

		if (wnd == this) {
			// ファイル登録
			GetCommandRepository()->RegisterCommandFromFiles(files);
		}
		else if (wnd == &in->mKeywordEdit) {
			// キーワードのEdit欄にドロップされた場合はパスをコピー

			for (auto& str : files) {
				if (in->mCommandStr.IsEmpty() == FALSE) {
					in->mCommandStr += _T(" ");
				}
				in->mCommandStr += str;
			}
			UpdateData(FALSE);
		}
		return 0;
	}

	UINT urlFormatId = RegisterClipboardFormat(CFSTR_INETURL);
	if (dataObj->IsDataAvailable(urlFormatId)) {

		STGMEDIUM st;
		if (dataObj->GetData(urlFormatId, &st) ) {
			CString urlString((LPCTSTR)GlobalLock(st.hGlobal));
			GlobalUnlock(st.hGlobal);

			if (wnd == this) {
				// URL登録
				launcherapp::core::CommandParameter param;
				param.SetNamedParamString(_T("TYPE"), _T("ShellExecCommand"));
				param.SetNamedParamString(_T("PATH"), urlString);

				GetCommandRepository()->NewCommandDialog(&param);
			}
			else if (wnd == &in->mKeywordEdit) {
				in->mCommandStr += urlString;
				UpdateData(FALSE);
			}

			return 0;
		}
	}
	return 0;
}

LRESULT
LauncherMainWindow::OnUserMessageCaptureWindow(WPARAM pParam, LPARAM lParam)
{
	SPDLOG_DEBUG(_T("start"));

	HWND hTargetWnd = (HWND)lParam;
	if (IsWindow(hTargetWnd) == FALSE) {
		return 0;
	}

	ProcessPath processPath(hTargetWnd);

	// 自プロセスのウインドウなら何もしない
	if (GetCurrentProcessId() == processPath.GetProcessId()) {
		return 0;
	}

	// 
	try {
		launcherapp::core::CommandParameter param;
		param.SetNamedParamString(_T("TYPE"), _T("ShellExecuteCommand"));
		param.SetNamedParamString(_T("COMMAND"), processPath.GetProcessName());
		param.SetNamedParamString(_T("PATH"), processPath.GetProcessPath());
		param.SetNamedParamString(_T("DESCRIPTION"), processPath.GetCaption());
		param.SetNamedParamString(_T("ARGUMENT"), processPath.GetCommandLine());

		GetCommandRepository()->NewCommandDialog(&param);
		return 0;
	}
	catch(ProcessPath::Exception& e) {
		CString errMsg((LPCTSTR)IDS_ERR_QUERYPROCESSINFO);
		CString pid;
		pid.Format(_T(" (PID:%d)"), e.GetPID());
		errMsg += pid;

		AfxMessageBox(errMsg);
		SPDLOG_ERROR((LPCTSTR)errMsg);
		return 0;
	}
}


LRESULT LauncherMainWindow::OnUserMessageHideAtFirst(
	WPARAM wParam,
	LPARAM lParam
)
{
	SPDLOG_DEBUG(_T("start"));

	HideWindow();
	return 0;
}

LRESULT LauncherMainWindow::OnUserMessageAppQuit(WPARAM wParam, LPARAM lParam)
{
	SPDLOG_DEBUG(_T("start"));

	PostQuitMessage(0);
	return 0;
}

LRESULT LauncherMainWindow::OnUserMessageSetClipboardString(
	WPARAM wParam,
 	LPARAM lParam
)
{
	SPDLOG_DEBUG(_T("start"));

	BOOL* isSetPtr = (BOOL*)wParam;
	HGLOBAL hMem = (HGLOBAL)lParam;

	::OpenClipboard(GetSafeHwnd());

	EmptyClipboard();

	UINT type = sizeof(TCHAR) == 2 ? CF_UNICODETEXT : CF_TEXT;
	SetClipboardData(type, hMem);
	CloseClipboard();

	if (isSetPtr) {
		*isSetPtr = TRUE;
	}

	return 0;
}

LRESULT LauncherMainWindow::OnUserMessageGetClipboardString(
	WPARAM wParam,
 	LPARAM lParam
)
{
	SPDLOG_DEBUG(_T("start"));

	CString* strPtr = (CString*)lParam;

	if (::OpenClipboard(GetSafeHwnd()) == FALSE) {
		SPDLOG_ERROR(_T("Failed to open clipboard."));
		return 0;
	}

	UINT type = sizeof(TCHAR) == 2 ? CF_UNICODETEXT : CF_TEXT;
	HANDLE hMem = GetClipboardData(type);
	if (hMem == NULL) {
		CloseClipboard();
		SPDLOG_WARN(_T("Clipboard is empty."));
		return 0;
	}

	LPTSTR p = (LPTSTR)GlobalLock(hMem);
	*strPtr = p;
	GlobalUnlock(hMem);

	CloseClipboard();

	return 0;
}

bool LauncherMainWindow::ExecuteCommand(const CString& str)
{
	SPDLOG_DEBUG(_T("args str:{}"), (LPCTSTR)str);

	launcherapp::core::CommandParameter commandParam(str);

	auto cmd = GetCommandRepository()->QueryAsWholeMatch(commandParam.GetCommandString(), true);
	if (cmd == nullptr) {
		SPDLOG_ERROR(_T("Command does not exist. name:{}"), (LPCTSTR)commandParam.GetCommandString());
		return false;
	}

	std::thread th([cmd, str]() {
		launcherapp::core::CommandParameter commandParam(str);
		cmd->Execute(commandParam);
		cmd->Release();
	});
	th.detach();

	return true;
}

// タスクトレイのダブルクリック時通知
LRESULT LauncherMainWindow::OnTaskTrayLButtonDblclk()
{
	SPDLOG_DEBUG(_T("start"));

	ActivateWindow();
	return 0;
}

// タスクトレイからのコンテキストメニュー表示依頼
LRESULT LauncherMainWindow::OnTaskTrayContextMenu(CWnd* wnd, CPoint point)
{
	SPDLOG_DEBUG(_T("start"));

	OnContextMenu(wnd, point);
	return 0;
}

void LauncherMainWindow::OnAppFirstBoot()
{
}

void LauncherMainWindow::OnAppNormalBoot()
{
}

void LauncherMainWindow::OnAppPreferenceUpdated()
{
	auto pref = AppPreference::Get();

	CString fontName;
	if (pref->GetMainWindowFontName(fontName) == false) {
		return;
	}

	LOGFONT lf;
	GetFont()->GetLogFont(&lf);

	if (in->mFontDlg.m_hObject) {
		in->mFontDlg.DeleteObject();
	}
	_tcsncpy_s(lf.lfFaceName, LF_FACESIZE, fontName, _TRUNCATE);
	in->mFontDlg.CreateFontIndirectW(&lf);
	SetFont(&in->mFontDlg);

	SendMessageToDescendants(WM_SETFONT, (WPARAM)in->mFontDlg.m_hObject, MAKELONG(FALSE, 0), FALSE);
}

void LauncherMainWindow::OnAppExit()
{
}

// LauncherMainWindow メッセージ ハンドラー

BOOL LauncherMainWindow::OnInitDialog()
{
	SPDLOG_DEBUG(_T("start"));

	CDialogEx::OnInitDialog();

	in->mKeyInputWatch.Create();

	auto pref = AppPreference::Get();

	CString fontName;
	if (pref->GetMainWindowFontName(fontName)) {
		LOGFONT lf;
		GetFont()->GetLogFont(&lf);

		_tcsncpy_s(lf.lfFaceName, LF_FACESIZE, fontName, _TRUNCATE);
		in->mFontDlg.CreateFontIndirectW(&lf);
		SetFont(&in->mFontDlg);

		SendMessageToDescendants(WM_SETFONT, (WPARAM)in->mFontDlg.m_hObject, MAKELONG(FALSE, 0), FALSE);
	}

	WTSRegisterSessionNotification(GetSafeHwnd(), NOTIFY_FOR_ALL_SESSIONS);
	SetTimer(TIMERID_OPERATION, 1000, nullptr);

	in->mOpWatcher.StartWatch(this);

	// グローバルホットキーのイベント受け取り先として登録する
	auto manager = core::CommandHotKeyManager::GetInstance();
	manager->SetReceiverWindow(GetSafeHwnd());

	in->mKeywordEdit.SubclassDlgItem(IDC_EDIT_COMMAND, this);
	in->mCmdReceiveEdit.SubclassDlgItem(IDC_EDIT_COMMAND2, this);
	in->mIconLabel.SubclassDlgItem(IDC_STATIC_ICON, this);
	in->mCandidateListBox.SubclassDlgItem(IDC_LIST_CANDIDATE, this);

	in->mCandidateListBox.InitColumns();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}


	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	HICON icon = IconLoader::Get()->LoadDefaultIcon();
	SetIcon(icon, TRUE);			// 大きいアイコンの設定
	SetIcon(icon, FALSE);		// 小さいアイコンの設定

	in->mSharedHwnd = std::make_unique<SharedHwnd>(GetSafeHwnd());

	in->mDescriptionStr = pref->GetDefaultComment();
	launcherapp::macros::core::MacroRepository::GetInstance()->Evaluate(in->mDescriptionStr);
	in->mGuideStr.Empty();

	// ウインドウ位置の復元
	in->RestoreWindowPosition(this, false);

	// 透明度制御
	in->mWindowTransparencyPtr->SetWindowHandle(GetSafeHwnd());

	// 設定値の読み込み
	GetCommandRepository()->Load();

	// ホットキー登録
	in->mHotKeyPtr = std::make_unique<AppHotKey>(GetSafeHwnd());
	if (in->mHotKeyPtr->Register() == false) {
		CString msg(_T("ホットキーを登録できませんでした。\n他のアプリケーションで使用されている可能性があります。\n"));
		msg += in->mHotKeyPtr->ToString();
		AfxMessageBox(msg);
		SPDLOG_ERROR(_T("Failed to restiser app hot key!"));
	}
	in->mMainWindowHotKeyPtr = std::make_unique<MainWindowHotKey>();
	in->mMainWindowHotKeyPtr->Register();
	
	UpdateData(FALSE);

	in->mDropTargetDialog.Register(this);
	in->mDropTargetEdit.Register(&in->mKeywordEdit);

	if (pref->IsHideOnStartup()) {
		PostMessage(WM_APP+7, 0, 0);
	}

	spdlog::info(_T("MainWindow initialized."));

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void LauncherMainWindow::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, IconLoader::Get()->LoadDefaultIcon());
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR LauncherMainWindow::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(IconLoader::Get()->LoadDefaultIcon());
}

LauncherMainWindow::CommandRepository*
LauncherMainWindow::GetCommandRepository()
{
	return CommandRepository::GetInstance();
}

void LauncherMainWindow::SetDescription(const CString& msg)
{
	in->mDescriptionStr = msg;
	UpdateData(FALSE);
}

void LauncherMainWindow::ClearContent()
{
	SPDLOG_DEBUG(_T("start"));

	AppPreference* pref= AppPreference::Get();
	in->mDescriptionStr = pref->GetDefaultComment();
	launcherapp::macros::core::MacroRepository::GetInstance()->Evaluate(in->mDescriptionStr);
	in->mGuideStr.Empty();

	in->mIconLabel.DrawDefaultIcon();
	in->mCommandStr.Empty();
	in->mCandidates.Clear();

	UpdateData(FALSE);
}

void LauncherMainWindow::Complement()
{
	WaitQueryRequest();

	auto cmd = GetCurrentCommand();
	if (cmd == nullptr) {
		spdlog::warn(_T("Comlement: bommand is null"));
		return ;
	}

	// 現在のキャレット位置より後ろにある文字を取得
	CString trailing;
	bool hasTrailing = in->GetTrailingString(trailing);

	in->mCommandStr = cmd->GetName();
	in->mCommandStr += _T(" ");

	if (hasTrailing) {
		in->mCommandStr += trailing;
	}

	UpdateData(FALSE);

	QuerySync();

	// 直前のQuerySyncの結果、選択中のコマンドが変化するため、取り直す
	cmd = GetCurrentCommand();
	if (cmd) {
		// コマンド名 + " " の位置にキャレットを設定する
		int caretPos = cmd->GetName().GetLength() + 1;
		in->mKeywordEdit.SetSel(caretPos, caretPos);
	}
}


// 現在選択中のコマンドを取得
core::Command*
LauncherMainWindow::GetCurrentCommand()
{
	return in->mCandidates.GetCurrentCommand();
}

/**
 * テキスト変更通知
 */
void LauncherMainWindow::OnEditCommandChanged()
{
	UpdateData();

	in->mLastInputStr = in->mCommandStr;

	// 音を鳴らす
	AppSound::Get()->PlayInputSound();

	// キー入力でCtrl-Backspaceを入力したとき、不可視文字(0x7E→Backspace)が入力される
	// (Editコントロールの通常の挙動)
	// このアプリはCtrl-Backspaceで入力文字列を全クリアするが、一方で、上記挙動により
	// 入力文字列をクリアした後、0x7Eが挿入されるという謎挙動になるので、ここで0x7Fを明示的に消している
	if (in->mCommandStr.Find((TCHAR)0x7F) != -1) {
		TCHAR bsStr[] = { (TCHAR)0x7F, (TCHAR)0x00 };
		in->mCommandStr.Replace(bsStr, _T(""));
		in->mKeywordEdit.Clear();
		ClearContent();
		return;
	}

	// 検索リクエスト
	QueryAsync();
}

// 入力キーワードで検索をリクエストを出す(完了をまたない)
void LauncherMainWindow::QueryAsync()
{
	// 入力テキストが空文字列の場合はデフォルト表示に戻す
	if (in->mCommandStr.IsEmpty()) {
		ClearContent();
		return;
	}

	// 検索リクエスト
	launcherapp::core::CommandParameter commandParam(in->mCommandStr);
	launcherapp::commands::core::CommandQueryRequest req(commandParam, GetSafeHwnd(), WM_APP+13);
	in->mIsQueryDoing = true;
	GetCommandRepository()->Query(req);
}


// 入力キーワードで検索をリクエストを出し、完了を待つ
void LauncherMainWindow::QuerySync()
{
	// 入力テキストが空文字列の場合はデフォルト表示に戻す
	if (in->mCommandStr.IsEmpty()) {
		ClearContent();
		return;
	}

	launcherapp::core::CommandParameter commandParam(in->mCommandStr);

	// キーワードによる絞り込みを実施
	launcherapp::commands::core::CommandQueryRequest req(commandParam, GetSafeHwnd(), WM_APP+13);
	in->mIsQueryDoing = true;
	GetCommandRepository()->Query(req);

	WaitQueryRequest();
}

// 候補欄を更新
void LauncherMainWindow::UpdateCandidates()
{
	// 入力テキストが空文字列の場合はデフォルト表示に戻す
	if (in->mCommandStr.IsEmpty()) {
		ClearContent();
		return;
	}

	// 候補なし
	if (in->mCandidates.IsEmpty()) {
		CString strMisMatch;
		strMisMatch.LoadString(ID_STRING_MISMATCH);
		in->mGuideStr.Empty();
		SetDescription(strMisMatch);
		in->mIconLabel.DrawIcon(IconLoader::Get()->LoadUnknownIcon());
		in->mCandidateListBox.Invalidate(TRUE);
		return;
	}
	else {
		CString descriptionStr = in->mCandidates.GetCurrentCommandDescription();
		// サムネイルの更新
		auto pCmd = GetCurrentCommand();
		ASSERT(pCmd);
		in->mIconLabel.DrawIcon(pCmd->GetIcon());
		in->mGuideStr = pCmd->GetGuideString();

		SetDescription(descriptionStr);
	}

	in->mCandidateListBox.Invalidate(TRUE);
}

void LauncherMainWindow::WaitQueryRequest()
{
	while(in->mIsQueryDoing) {
		MSG msg;
		GetMessage(&msg, 0, NULL, NULL);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

/**
 	コマンドを実行する
 	@param[in] cmd 実行対象のコマンドオブジェクト
*/
void LauncherMainWindow::RunCommand(
	launcherapp::core::Command* cmd
)
{
	// 実行時の音声を再生する
	AppSound::Get()->PlayExecuteSound();

	CString str = in->mCommandStr;

	// コマンドの参照カウントを上げる(実行完了時に下げる)
	cmd->AddRef();

	std::thread th([cmd, str]() {

		launcherapp::core::CommandParameter commandParam(str);

		// Ctrlキーが押されているかを設定
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
			commandParam.SetNamedParamBool(_T("CtrlKeyPressed"), true);
		}
		if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
			commandParam.SetNamedParamBool(_T("ShiftKeyPressed"), true);
		}
		if (GetAsyncKeyState(VK_LWIN) & 0x8000) {
			commandParam.SetNamedParamBool(_T("WinKeyPressed"), true);
		}
		if (GetAsyncKeyState(VK_MENU) & 0x8000) {
			commandParam.SetNamedParamBool(_T("AltKeyPressed"), true);
		}

		if (cmd->Execute(commandParam) == FALSE) {
			auto errMsg = cmd->GetErrorString();
			if (errMsg.IsEmpty() == FALSE) {
				auto app = (LauncherApp*)AfxGetApp();
				app->PopupMessage(errMsg);
			}
		}

		// コマンドの参照カウントを下げる
		cmd->Release();
	});
	th.detach();
}


void LauncherMainWindow::OnOK()
{
	UpdateData();

	WaitQueryRequest();

	auto cmd = GetCurrentCommand();
	if (cmd) {
		// コマンドを実行する
		RunCommand(cmd);
	}
	else {
		// 空文字状態でEnterキーから実行したときはキーワードマネージャを表示
		if (in->mCommandStr.IsEmpty()) {
			ExecuteCommand(_T("manager"));
		}
	}
	// 実行したら、入力文字列を消して、入力ウインドウも非表示にする
	ClearContent();
	HideWindow();
}

void LauncherMainWindow::OnCancel()
{
	// 入力欄に入力中のテキストがあったらクリア、何もなければメインウインドウを非表示にする
	if (in->mCommandStr.IsEmpty() == FALSE) {
		ClearContent();
		GetDlgItem(IDC_EDIT_COMMAND)->SetFocus();
	}
	else {
		HideWindow();
	}
}

LRESULT LauncherMainWindow::WindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_HOTKEY) {
		// ホットキー押下からの表示状態変更
		if (in->mHotKeyPtr->IsSameKey(lp)) {
			// アプリ呼び出しホットキー
			ActivateWindow(GetSafeHwnd());
		}
		else {
			// コマンド実行ホットキー
			auto manager = core::CommandHotKeyManager::GetInstance();
			manager->InvokeGlobalHandler(lp);
		}
		return 0;
	}
	return CDialogEx::WindowProc(msg, wp, lp);
}

BOOL LauncherMainWindow::PreTranslateMessage(MSG* pMsg)
{
	HACCEL accel = core::CommandHotKeyManager::GetInstance()->GetAccelerator();
	if (accel && TranslateAccelerator(GetSafeHwnd(), accel, pMsg)) {
		return TRUE;
	}
	return __super::PreTranslateMessage(pMsg);
}

void LauncherMainWindow::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if (bShow) {
		// 「隠れるときに入力文字列を消去しない」設定に応じてテキストを消す
		// (実際には表示直前に消去する)
		AppPreference* pref = AppPreference::Get();
		if (pref->IsKeepTextWhenDlgHide() == false) {
			ClearContent();
		}
		GetDlgItem(IDC_EDIT_COMMAND)->SetFocus();

		SetTimer(TIMERID_KEYSTATE, 375, nullptr);
	}
	else {
		// 位置情報を更新する
		if (in->mWindowPositionPtr.get()) {
			in->mWindowPositionPtr->Update(GetSafeHwnd());
		}

		KillTimer(TIMERID_KEYSTATE);
	}
}


/**
 *
 */
LRESULT LauncherMainWindow::OnKeywordEditNotify(
	WPARAM wParam,
	LPARAM lParam
)
{
	if (wParam == VK_BACK) {
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {

			AppSound::Get()->PlayInputSound();

			ClearContent();
			in->mKeywordEdit.Clear();
			return 1;
		}
	}

	if (in->mCandidates.IsEmpty() == false) {

		// 矢印↑キー押下
		if (wParam == VK_UP) {
			in->mCandidates.OffsetCurrentSelect(-1, true);

			auto cmd = GetCurrentCommand();
			if (cmd == nullptr) {
				spdlog::debug(_T("command is null vk:{}"), wParam);
				return 1;
			}

			AppSound::Get()->PlaySelectSound();

			int startPos = 0;
			int endPos = 0;
			in->UpdateCommandString(cmd, startPos, endPos);
			UpdateData(FALSE);

			in->mKeywordEdit.SetSel(startPos, endPos);

			return 1;
		}
		// 矢印↓キー押下
		else if (wParam ==VK_DOWN) {
			in->mCandidates.OffsetCurrentSelect(1, true);

			auto cmd = GetCurrentCommand();
			if (cmd == nullptr) {
				spdlog::debug(_T("command is null vk:{}"), wParam);
				return 1;
			}

			AppSound::Get()->PlaySelectSound();

			int startPos = 0;
			int endPos = 0;
			in->UpdateCommandString(cmd, startPos, endPos);
			UpdateData(FALSE);

			in->mKeywordEdit.SetSel(startPos, endPos);

			return 1;
		}
		else if (wParam == VK_TAB) {
			// 補完
			Complement();
			return 1;

		}
		else if (wParam == VK_RETURN) {
			OnOK();
			return 1;
		}
		else if (wParam == VK_NEXT) {
			int itemsInPage = in->mCandidateListBox.GetItemCountInPage();
			in->mCandidates.OffsetCurrentSelect(itemsInPage, false);
			return 1;
		}
		else if (wParam == VK_PRIOR) {
			int itemsInPage = in->mCandidateListBox.GetItemCountInPage();
			in->mCandidates.OffsetCurrentSelect(-itemsInPage , false);
			return 1;
		}
	}

	return 0;
}

// クライアント領域をドラッグしてウインドウを移動させるための処理
LRESULT LauncherMainWindow::OnNcHitTest(
	CPoint point
)
{

	RECT rect;
	GetClientRect (&rect);

	CPoint ptClient(point);
	ScreenToClient(&ptClient);

	if (PtInRect(&rect, ptClient) && (GetAsyncKeyState( VK_LBUTTON ) & 0x8000) )
	{
		return HTCAPTION;
	}
	return __super::OnNcHitTest(point);
}

// SetItemStateで状態設定されると、この通知が呼ばれる
void LauncherMainWindow::OnLvnItemChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;
	if (nm->iItem == -1) {
		return;
	}

	AppSound::Get()->PlaySelectSound();

	in->mCandidates.SetCurrentSelect(nm->iItem);
	UpdateData(FALSE);

}

// 候補欄のリストをクリックしたときの処理
// 選択した要素の情報を入力欄やコメント欄に反映する
void LauncherMainWindow::OnNMClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	 NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;
	 if (nm->iItem == -1) {
		 return;
	 }

	 auto cmd = GetCurrentCommand();
	 if (cmd == nullptr) {
		 spdlog::warn(_T("command is null. iItem:{}"), nm->iItem);
		 return ;
	 }

	 // 選択したコマンドの情報を入力欄やコメント欄に反映する
	 int startPos = 0;
	 int endPos = 0;
	 in->UpdateCommandString(cmd, startPos, endPos);
	 UpdateData(FALSE);

	 // 入力欄を選択状態にする
	 in->mKeywordEdit.SetSel(startPos, endPos);
	 in->mKeywordEdit.SetFocus();
}

void LauncherMainWindow::OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	 NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;
	// ダブルクリックで確定
	OnOK();
}

void LauncherMainWindow::OnSize(UINT type, int cx, int cy)
{
	__super::OnSize(type, cx, cy);

	if (in->mCandidateListBox.GetSafeHwnd()) {
		in->mCandidateListBox.UpdateSize(cx, cy);
	}
}

/**
 * コンテキストメニューの表示
 */
void LauncherMainWindow::OnContextMenu(
	CWnd* pWnd,
	CPoint point
)
{
	SPDLOG_DEBUG(_T("args point:({0},{1})"), point.x, point.y);

	CMenu menu;
	menu.CreatePopupMenu();

	const int ID_SHOW = 1;
	const int ID_HIDE = 2;
	const int ID_NEW = 3;
	const int ID_MANAGER = 4;
	const int ID_APPSETTING = 5;
	const int ID_USERDIR = 6;
	const int ID_RESETPOS = 7;
	const int ID_MANUAL = 8;
	const int ID_VERSIONINFO = 9;
	const int ID_EXIT = 19;

	BOOL isVisible = IsWindowVisible();
	CString textToggleVisible(isVisible ? (LPCTSTR)IDS_MENUTEXT_HIDE : (LPCTSTR)IDS_MENUTEXT_SHOW);

	menu.InsertMenu(-1, 0, isVisible ? ID_HIDE : ID_SHOW, textToggleVisible);
	menu.InsertMenu(-1, MF_SEPARATOR, 0, _T(""));
	menu.InsertMenu(-1, 0, ID_APPSETTING, _T("アプリケーションの設定(&S)"));
	menu.InsertMenu(-1, 0, ID_NEW, _T("新規作成(&N)"));
	menu.InsertMenu(-1, 0, ID_MANAGER, _T("キーワードマネージャ(&K)"));
	menu.InsertMenu(-1, MF_SEPARATOR, 0, _T(""));
	menu.InsertMenu(-1, 0, ID_USERDIR, _T("設定フォルダを開く(&D)"));
	menu.InsertMenu(-1, 0, ID_RESETPOS, _T("ウインドウ位置をリセット(&R)"));
	menu.InsertMenu(-1, MF_SEPARATOR, 0, _T(""));
	menu.InsertMenu(-1, 0, ID_MANUAL, _T("ヘルプ(&H)"));
	menu.InsertMenu(-1, 0, ID_VERSIONINFO, _T("バージョン情報(&V)"));
	menu.InsertMenu(-1, MF_SEPARATOR, 0, _T(""));
	menu.InsertMenu(-1, 0, ID_EXIT, _T("終了(&E)"));

	spdlog::info(_T("Show context menu."));

	// TrackPopupMenuをよぶ時点でSetForegroundWindowでウインドウをアクティブにしておかないと
	// ポップアップメニュー以外の領域をクリックしたときにメニューが閉じないので、ここで呼んでおく
	SetForegroundWindow();

	int n = menu.TrackPopupMenu(TPM_RETURNCMD, point.x, point.y, this);
	spdlog::debug(_T("selected menu id:{}"), n);

	if (n == ID_SHOW) {
		ActivateWindow();
	}
	else if (n == ID_HIDE) {
		HideWindow();
	}
	else if (n == ID_NEW) {
		ExecuteCommand(_T("new"));
	}
	else if (n == ID_MANAGER) {
		ExecuteCommand(_T("manager"));
	}
	else if (n == ID_APPSETTING) {
		ExecuteCommand(_T("setting"));
	}
	else if (n == ID_USERDIR) {
		ExecuteCommand(_T("userdir"));
	}
	else if (n == ID_RESETPOS) {
		// ウインドウ位置をリセット
		in->RestoreWindowPosition(this, true);
	}
	else if (n == ID_MANUAL) {
		// ヘルプ表示
		ShowHelpTop();
	}
	else if (n == ID_VERSIONINFO) {
		ExecuteCommand(_T("version"));
	}
	else if (n == ID_EXIT) {
		ExecuteCommand(_T("exit"));
	}
}

void LauncherMainWindow::OnActivate(UINT nState, CWnd* wnd, BOOL bMinimized)
{
	if (in->mIsBlockDeactivateOnUnfocus == false) {
		in->mWindowTransparencyPtr->UpdateActiveState(nState);
	}
	__super::OnActivate(nState, wnd, bMinimized);
}

// Windowsの終了(ログオフ)通知
void LauncherMainWindow::OnEndSession(BOOL isEnding)
{
	SPDLOG_INFO(_T("args isEnding:{}"), (bool)isEnding);

	if (isEnding) {
		PostQuitMessage(0);
	}
}

void LauncherMainWindow::OnCommandHelp()
{
	// ヘルプ表示
	ShowHelpInputWindow();
}

void LauncherMainWindow::OnCommandHotKey(UINT id)
{
	// ローカルホットキーに対応されたコマンドを実行する
	SPDLOG_DEBUG(_T("args id:{}"), id);
	core::CommandHotKeyManager::GetInstance()->InvokeLocalHandler(id);
}

void LauncherMainWindow::OnTimer(UINT_PTR timerId)
{
	if (timerId == TIMERID_KEYSTATE) {
		// いったん無効化
		// bool shouldBeTransparent = (GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(VK_MENU) & 0x8000);
		// if (in->mIsPrevTransparentState != shouldBeTransparent) {
		// 	in->mWindowTransparencyPtr->ToggleAlphaState(shouldBeTransparent);
		// 	in->mIsPrevTransparentState = shouldBeTransparent;
		// }
	}
	else if (timerId == TIMERID_OPERATION) {
		LauncherWindowEventDispatcher::Get()->Dispatch([](LauncherWindowEventListenerIF* listener) {
			listener->OnTimer();
		});
	}
}

LRESULT LauncherMainWindow::OnMessageSessionChange(WPARAM wParam, LPARAM lParam)
{
	// ロックされたとき、wpにWTS_SESSION_LOCK(7)、解除されたときは WTS_SESSION_UNLOCK(8)が通知される
	if (wParam == WTS_SESSION_LOCK) {
		SPDLOG_INFO(_T("WTS_SESSION_LOCK"));
		LauncherWindowEventDispatcher::Get()->Dispatch([](LauncherWindowEventListenerIF* listener) {
				listener->OnLockScreenOccurred();
		});
	}
	else if (wParam == WTS_SESSION_UNLOCK) {
		SPDLOG_INFO(_T("WTS_SESSION_UNLOCK"));
		LauncherWindowEventDispatcher::Get()->Dispatch([](LauncherWindowEventListenerIF* listener) {
				listener->OnUnlockScreenOccurred();
		});
	}
	return 0;
}
