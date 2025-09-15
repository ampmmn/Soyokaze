
// LauncherMainWindow.cpp : 実装ファイル
//

#include "pch.h"
#include "framework.h"
#include "app/LauncherApp.h"
#include "app/Manual.h"
#include "mainwindow/LauncherMainWindow.h"
#include "mainwindow/CandidateListCtrl.h"
#include "mainwindow/layout/MainWindowLayout.h"
#include "mainwindow/layout/MainWindowAppearance.h"
#include "mainwindow/MainWindowInput.h"
#include "mainwindow/controller/LauncherMainWindowController.h"
#include "mainwindow/MainWindowCommandQueryRequest.h"
#include "mainwindow/optionbutton/MainWindowOptionButton.h"
#include "core/IFIDDefine.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "commands/core/ContextMenuSourceIF.h"
#include "commands/core/SelectionBehavior.h"
#include "commands/common/Clipboard.h"
#include "afxdialogex.h"
#include "utility/Path.h"
#include "SharedHwnd.h"
#include "hotkey/AppHotKey.h"
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/KeyInputWatch.h"
#include "icon/IconLoader.h"
#include "setting/AppPreference.h"
#include "mainwindow/AppSound.h"
#include "mainwindow/LauncherWindowEventDispatcher.h"
#include "utility/ProcessPath.h"
#include "utility/ScopeAttachThreadInput.h"
#include "mainwindow/MainWindowHotKey.h"
#include "mainwindow/OperationWatcher.h"
#include "macros/core/MacroRepository.h"
#include "matcher/CommandToken.h"
#include "mainwindow/CandidateList.h"
#include <algorithm>
#include <thread>

#include <wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")

#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp;
using namespace launcherapp::mainwindow;
using namespace launcherapp::mainwindow::controller;

// ランチャーのタイマーイベントをリスナーに通知する用のタイマー
constexpr UINT TIMERID_OPERATION = 2;

struct LauncherMainWindow::PImpl
{
	PImpl(LauncherMainWindow* thisPtr) : 
		mDropTargetDialog(thisPtr),
		mDropTargetEdit(thisPtr)
	{
	}

	void UpdateCommandString(core::Command* cmd, int& startPos, int& endPos);

// 入力データ
	// 入力欄のテキスト情報
	MainWindowInput mInput;
	// 最後に外部からの入力によって更新された時点での文字列
	CString mLastInputStr;
	CString mGuideStr;
	// 現在選択中のコマンドの説明
	CString mDescriptionStr;
	// 現在の候補一覧(選択中の項目もここが管理する)
	CandidateList mCandidates;

// キーワード検索状態
	bool mIsQueryDoing{false};

	// ウインドウハンドル(共有メモリに保存する用)
	std::unique_ptr<SharedHwnd> mSharedHwnd;
	   // 後で起動したプロセスから有効化するために共有メモリに保存している

// ウインドウ上に配置する部品
	// キーワード入力エディットボックス
	KeywordEdit mKeywordEdit;
	// アイコン描画用ラベル
	CaptureIconLabel mIconLabel;
	// 候補一覧表示用リストボックス
	CandidateListCtrl mCandidateListBox;
	// オプションボタン
	MainWindowOptionButton mOptionButton;

// ウインドウ状態管理
	// 位置・サイズ・コンポーネントの配置を管理するクラス
	std::unique_ptr<MainWindowLayout> mLayout;
	// 外観(色、フォント)などを管理するクラス
	std::unique_ptr<MainWindowAppearance> mAppearance;
	// 表示を抑制中か
	bool mIsWindowDisplayBlocked{false};

// ホットキー関連
	// 入力画面を呼び出すホットキー関連の処理をする
	std::unique_ptr<AppHotKey> mHotKeyPtr;
	std::unique_ptr<MainWindowHotKey> mMainWindowHotKeyPtr;
	// キー入力監視(修飾キー入力によるホットキー機能用)
	KeyInputWatch mKeyInputWatch;

// 外部との連携用
	// 外部からのコマンド受付用エディットボックス
	CmdReceiveEdit mCmdReceiveEdit;
	// ドロップターゲット
	LauncherDropTarget mDropTargetDialog;
	LauncherDropTarget mDropTargetEdit;

// その他
	// 稼働状況を監視する(長時間連続稼働を警告する目的)
	OperationWatcher mOpWatcher;

};

// 候補欄の選択変更により、新しく選択されたコマンドの名前で選択欄のテキストを置き換える
void LauncherMainWindow::PImpl::UpdateCommandString(core::Command* cmd, int& startPos, int& endPos)
{
	spdlog::debug("UpdateCommandString");

	// コマンド名
	auto cmdName = cmd->GetName();

	// コマンド名が前回の入力ワードと前方一致し、文字列が増えていたら
	if (cmdName.Find(mLastInputStr) == 0 && cmdName.GetLength() > mLastInputStr.GetLength()) {
		// 
		mInput.SetKeyword(cmdName);
		// 追記部分を選択した状態にする
		startPos = mLastInputStr.GetLength();
		endPos = cmdName.GetLength();
	}
	else {
		// 上記に該当しない場合は、前回の状態から変更しない
		mInput.SetKeyword(mLastInputStr);
		// (キャレット選択もしない)
		startPos = mInput.GetLength();
		endPos = mInput.GetLength();
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


// LauncherMainWindow ダイアログ

LauncherMainWindow::LauncherMainWindow(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MAIN, pParent),
	in(std::make_unique<PImpl>(this))
{
	in->mLayout = std::make_unique<MainWindowLayout>(this);

	in->mCandidateListBox.SetCandidateList(&in->mCandidates);
}

LauncherMainWindow::~LauncherMainWindow()
{
	in->mCandidates.RemoveListener(&in->mCandidateListBox);
}

void LauncherMainWindow::DoDataExchange(CDataExchange* pDX)
{
	auto& commandStr = in->mInput.CommandStr();

	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COMMAND, commandStr);
	DDX_Text(pDX, IDC_STATIC_GUIDE, in->mGuideStr);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, in->mDescriptionStr);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(LauncherMainWindow, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDIT_COMMAND, OnEditCommandChanged)
	ON_WM_SHOWWINDOW()
	ON_WM_NCHITTEST()
	ON_WM_ACTIVATE()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_CANDIDATE, OnLvnItemChange)
	ON_NOTIFY(NM_CLICK, IDC_LIST_CANDIDATE, OnNMClick)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CANDIDATE, OnNMDblclk)
	ON_WM_SIZING()
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_MBUTTONUP()
	ON_MESSAGE(LauncherMainWindowMessageID::INPUTKEY, OnKeywordEditNotify)
	ON_MESSAGE(LauncherMainWindowMessageID::ACTIVATEWINDOW, OnUserMessageActiveWindow)
	ON_MESSAGE(LauncherMainWindowMessageID::RUNCOMMAND, OnUserMessageRunCommand)
	ON_MESSAGE(WM_APP+4, OnUserMessageDragOverObject)
	ON_MESSAGE(WM_APP+5, OnUserMessageDropObject)
	ON_MESSAGE(WM_APP+6, OnUserMessageCaptureWindow)
	ON_MESSAGE(LauncherMainWindowMessageID::HIDEWINDOW, OnUserMessageHide)
	ON_MESSAGE(LauncherMainWindowMessageID::QUITAPPLICATION, OnUserMessageAppQuit)
	ON_MESSAGE(LauncherMainWindowMessageID::SETCLIPBOARDSTRING, OnUserMessageSetClipboardString)
	ON_MESSAGE(LauncherMainWindowMessageID::GETCLIPBOARDSTRING, OnUserMessageGetClipboardString)
	ON_MESSAGE(LauncherMainWindowMessageID::SETTEXT, OnUserMessageSetText)
	ON_MESSAGE(WM_APP+12, OnUserMessageSetSel)
	ON_MESSAGE(WM_APP+13, OnUserMessageQueryComplete)
	ON_MESSAGE(LauncherMainWindowMessageID::BLOCKDEACTIVATE, OnUserMessageBlockDeactivateOnUnfocus)
	ON_MESSAGE(LauncherMainWindowMessageID::UPDATECANDIDATE, OnUserMessageUpdateCandidate)
	ON_MESSAGE(LauncherMainWindowMessageID::COPYINPUTTEXT, OnUserMessageCopyText)
	ON_MESSAGE(LauncherMainWindowMessageID::REQUESTCALLBACK, OnUserMessageRequestCallback)
	ON_MESSAGE(WM_APP+18, OnUserMessageClearContent)
	ON_MESSAGE(LauncherMainWindowMessageID::MOVETEMPORARY, OnUserMessageMoveTemporary)
	ON_MESSAGE(LauncherMainWindowMessageID::BLOCKWINDOWDIAPLAY, OnUserMessageBlockWindowDiaplay)
	ON_WM_CONTEXTMENU()
	ON_WM_ENDSESSION()
	ON_WM_TIMER()
	ON_COMMAND(ID_HELP, OnCommandHelp)
	ON_MESSAGE(WM_WTSSESSION_CHANGE, OnMessageSessionChange)
	ON_WM_CTLCOLOR()
	ON_WM_MEASUREITEM()
	ON_BN_CLICKED(IDC_BUTTON_OPTION, OnButtonOptionClicked)
	ON_COMMAND_RANGE(core::CommandHotKeyManager::ID_LOCAL_START, 
	                 core::CommandHotKeyManager::ID_LOCAL_END, OnCommandHotKey)
END_MESSAGE_MAP()

#pragma warning( pop )

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
	LauncherWindowEventDispatcher::Get()->Dispatch([](LauncherWindowEventListenerIF* listener) {
		listener->OnLauncherUnactivate();
	});
	in->mLayout->HideWindow();
}


void LauncherMainWindow::ShowHelpTop()
{
	SPDLOG_DEBUG("start");
	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate("Top");
}


// 現在のスレッドのウインドウで最も前面にあるウインドウハンドルを取得する
static HWND GetTopMostWindowInCurrentThread()
{
	HWND hTopMost = nullptr;
	EnumThreadWindows(GetCurrentThreadId(), [](HWND h, LPARAM param) -> BOOL {
		if (IsWindowVisible(h) == FALSE) {
			return TRUE;
		}

		HWND* pTopMost = reinterpret_cast<HWND*>(param);
		if (*pTopMost == nullptr) {
			// 初回はとりあえず拾っておく
			*pTopMost = h;
			return TRUE;
		}
	 	if (GetWindow(h, GW_HWNDPREV) == nullptr) {
			*pTopMost = h;
			return FALSE; // 最前面が見つかったので列挙終了
		}
		return TRUE;
	}, reinterpret_cast<LPARAM>(&hTopMost));

	return hTopMost;
}

/**
 * ActiveWindow経由の処理
 * (後続プロセスから処理できるようにするためウインドウメッセージ経由で処理している)
 */
LRESULT LauncherMainWindow::OnUserMessageActiveWindow(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	bool isShowForce = ((wParam & 0x1) != 0);

	HWND hwnd = GetSafeHwnd();
	if (isShowForce || ::IsWindowVisible(hwnd) == FALSE) {

		ScopeAttachThreadInput scope;

		// もし外部からメインウインドウの表示が抑制状態である場合は表示しない
		// (現在、設定画面表示中のみ抑制する)
		if (in->mIsWindowDisplayBlocked) {
			auto h = GetTopMostWindowInCurrentThread();
			::SetForegroundWindow(h);
			return 0;
		}

		// 非表示状態なら表示

		// 表示する際の位置を決定(移動)する
		in->mLayout->RecalcWindowOnActivate(this);

		// プレースホルダー設定
		AppPreference* pref= AppPreference::Get();
		LPCTSTR placeholderText = pref->IsDrawPlaceHolder() ? _T("キーワードを入力してください") : _T("");
		in->mKeywordEdit.SetPlaceHolder(placeholderText);

		// 表示
		::ShowWindow(hwnd, SW_SHOW);
		::SetForegroundWindow(hwnd);
		::BringWindowToTop(hwnd);

		if (pref->IsIMEOffOnActive()) {
			in->mKeywordEdit.SetIMEOff();
		}

		LauncherWindowEventDispatcher::Get()->Dispatch([](LauncherWindowEventListenerIF* listener) {
			listener->OnLauncherActivate();
		});
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

	bool isWaitSync = (wParam == 1);

	if(isWaitSync) {
		// 入力欄にテキストを入力して、検索をまって、先頭の候補を実行する
		OnUserMessageSetText(0, lParam);
		OnOK();
		return 0;
	}
	else {
		// 単にテキストに合致するコマンドを実行するだけ
		LPCTSTR text = (LPCTSTR)lParam;
		if (text == nullptr) {
			SPDLOG_WARN(_T("text is null"));
			return 0;
		}
		ExecuteCommand(text);
		return 0;
	}
}

/**
 * 入力欄にテキストをセットする処理
 */
LRESULT LauncherMainWindow::OnUserMessageSetText(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);

	SPDLOG_DEBUG(_T("start"));

	LPCTSTR text = (LPCTSTR)lParam;
	if (text == nullptr) {
		SPDLOG_WARN(_T("text is null"));
		return 0;
	}
	SPDLOG_DEBUG(_T("text:{}"), text);

	in->mInput.SetKeyword(text);
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
	UNREFERENCED_PARAMETER(wParam);

	in->mIsQueryDoing = false;
	auto result = (launcherapp::commands::core::CommandQueryResult*)lParam;
	if (result != nullptr) {

		int matchLevel = Pattern::Mismatch;

		std::vector<launcherapp::core::Command*> commands;
		size_t count = result->GetCount();
		for (size_t i = 0; i < count; ++i) {
			matchLevel = Pattern::Mismatch;
			launcherapp::core::Command* cmd = nullptr;
			if (result->Get(i, &cmd, &matchLevel) == false) {
				continue;
			}
			//cmd->AddRef();  // Getにより参照カウントは+1されるため、ここでは不要
			commands.push_back(cmd);
		}
		result->Release();

		// 自動実行を許可する場合は実行する
		bool canAutoExecute = commands.size() == 1 && matchLevel == Pattern::WholeMatch;		if (canAutoExecute && commands[0]->IsAllowAutoExecute()) {
			RunCommand(commands[0]);
			return 0;
		}

		in->mCandidates.SetItems(commands);

	}
	else {
		in->mCandidates.Clear();
	}

	UpdateCandidates();

	return 0;
}

LRESULT LauncherMainWindow::OnUserMessageBlockDeactivateOnUnfocus(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);

	in->mAppearance->SetBlockDeactivateOnUnfocus(lParam != 0);

	return 0;
}

LRESULT LauncherMainWindow::OnUserMessageUpdateCandidate(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	if (::IsWindowVisible(GetSafeHwnd()) == FALSE) {
		// ウインドウを表示していない場合は更新しない
		return 0;
	}

	// 候補欄を更新するため、再度検索リクエストを出す
	QueryAsync();
	return 0;
}

LRESULT LauncherMainWindow::OnUserMessageCopyText(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	// 入力欄のテキストをコピー
	launcherapp::commands::common::Clipboard::Copy(in->mInput.GetKeyword());
	return 0;
}

LRESULT LauncherMainWindow::OnUserMessageRequestCallback(WPARAM wParam, LPARAM lParam)
{
	typedef LRESULT(*LAUNCHERWINDOWCALLBACK)(LPARAM lp);
	LAUNCHERWINDOWCALLBACK callbackFunc = (LAUNCHERWINDOWCALLBACK)wParam;
	return callbackFunc(lParam);
}

LRESULT LauncherMainWindow::OnUserMessageClearContent(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	ClearContent();
	return 0;
}

LRESULT LauncherMainWindow::OnUserMessageMoveTemporary(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	// VK_UP/DOWN/LEFT/RIGHTを移動の方向として使う(手抜き)
	int vk = (int)wParam;
	return in->mLayout->MoveTemporary(vk) ? 0 : 1;
}

LRESULT LauncherMainWindow::OnUserMessageBlockWindowDiaplay(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	bool isBlock = wParam != 0;
	in->mIsWindowDisplayBlocked = isBlock;
	return 0;
}

void LauncherMainWindow::OnButtonOptionClicked()
{
	ExecuteCommand(_T("setting"));
}

LRESULT 
LauncherMainWindow::OnUserMessageDragOverObject(
	WPARAM wParam,
 	LPARAM lParam
)
{
	UNREFERENCED_PARAMETER(wParam);

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

			Path filePath;
			for (int i = 0; i < fileCount; ++i) {
				DragQueryFile(dropInfo, i, filePath, (UINT)filePath.size());
				files.push_back((LPCTSTR)filePath);
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
				in->mInput.AddArgument(str);
			}
			UpdateData(FALSE);
		}
		return 0;
	}

	UINT urlFormatId = RegisterClipboardFormat(CFSTR_INETURL);
	if (dataObj->IsDataAvailable((CLIPFORMAT)urlFormatId)) {

		STGMEDIUM st;
		if (dataObj->GetData((CLIPFORMAT)urlFormatId, &st) ) {
			CString urlString((LPCTSTR)GlobalLock(st.hGlobal));
			GlobalUnlock(st.hGlobal);

			if (wnd == this) {
				// URL登録
				auto param = launcherapp::core::CommandParameterBuilder::Create();
				param->SetNamedParamString(_T("TYPE"), _T("ShellExecCommand"));
				param->SetNamedParamString(_T("PATH"), urlString);

				GetCommandRepository()->NewCommandDialog(param);

				param->Release();
			}
			else if (wnd == &in->mKeywordEdit) {
				in->mInput.AddArgument(urlString);
				UpdateData(FALSE);
			}

			return 0;
		}
	}
	return 0;
}

/**
 	アイコン欄をドラッグして他ウインドウをキャプチャしたときに実行されるハンドラ
 	@return 0
 	@param[in] wParam  0
 	@param[in] lParam  キャプチャ対象ウインドウハンドル
*/
LRESULT
LauncherMainWindow::OnUserMessageCaptureWindow(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);

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
		auto param = launcherapp::core::CommandParameterBuilder::Create();
		param->SetNamedParamString(_T("TYPE"), _T("ShellExecuteCommand"));
		param->SetNamedParamString(_T("COMMAND"), processPath.GetProcessName());
		param->SetNamedParamString(_T("PATH"), processPath.GetProcessPath());
		param->SetNamedParamString(_T("DESCRIPTION"), processPath.GetCaption());
		param->SetNamedParamString(_T("ARGUMENT"), processPath.GetCommandLine());

		GetCommandRepository()->NewCommandDialog(param);

		param->Release();
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


LRESULT LauncherMainWindow::OnUserMessageHide(
	WPARAM wParam,
	LPARAM lParam
)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	SPDLOG_DEBUG(_T("start"));

	HideWindow();
	return 0;
}

LRESULT LauncherMainWindow::OnUserMessageAppQuit(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

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
	UNREFERENCED_PARAMETER(wParam);

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

	auto commandParam = launcherapp::core::CommandParameterBuilder::Create(str);

	auto cmd = GetCommandRepository()->QueryAsWholeMatch(commandParam->GetCommandString(), true);
	if (cmd == nullptr) {
		SPDLOG_ERROR(_T("Command does not exist. name:{}"), (LPCTSTR)commandParam->GetCommandString());

		commandParam->Release();
		return false;
	}

	RunCommand(cmd, commandParam);
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

CWnd* LauncherMainWindow::GetWindowObject()
{
	return this;
}

IconLabel* LauncherMainWindow::GetIconLabel()
{
	return &in->mIconLabel;
}

CStatic* LauncherMainWindow::GetDescriptionLabel()
{
	return (CStatic*)GetDlgItem(IDC_STATIC_DESCRIPTION);
}

CStatic* LauncherMainWindow::GetGuideLabel()
{
	return (CStatic*)GetDlgItem(IDC_STATIC_GUIDE);
}

KeywordEdit* LauncherMainWindow::GetEdit()
{
	return &in->mKeywordEdit;
}

CandidateListCtrl* LauncherMainWindow::GetCandidateList()
{
	return &in->mCandidateListBox;
}

CFont* LauncherMainWindow::GetMainWindowFont()
{
	return in->mAppearance->GetFont();
}

// LauncherMainWindow メッセージ ハンドラー

BOOL LauncherMainWindow::OnInitDialog()
{
	SPDLOG_DEBUG(_T("start"));

	CDialogEx::OnInitDialog();

	in->mAppearance = std::make_unique<MainWindowAppearance>(this);
	in->mKeyInputWatch.Create();

	// スクリーンロック/アンロックの通知をうけとる
	WTSRegisterSessionNotification(GetSafeHwnd(), NOTIFY_FOR_ALL_SESSIONS);

	SetTimer(TIMERID_OPERATION, 1000, nullptr);

	in->mOpWatcher.StartWatch(this);

	// グローバルホットキーのイベント受け取り先として登録する
	auto manager = core::CommandHotKeyManager::GetInstance();
	manager->SetReceiverWindow(GetSafeHwnd());

	in->mKeywordEdit.SubclassDlgItem(IDC_EDIT_COMMAND, this);

	in->mCmdReceiveEdit.SubclassDlgItem(IDC_EDIT_COMMAND2, this);
	in->mCmdReceiveEdit.Init();

	in->mIconLabel.SubclassDlgItem(IDC_STATIC_ICON, this);

	in->mCandidateListBox.SubclassDlgItem(IDC_LIST_CANDIDATE, this);
	in->mCandidateListBox.InitColumns();

	in->mOptionButton.SubclassDlgItem(IDC_BUTTON_OPTION, this);

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

	auto pref = AppPreference::Get();
	in->mDescriptionStr = pref->GetDefaultComment();
	launcherapp::macros::core::MacroRepository::GetInstance()->Evaluate(in->mDescriptionStr);
	in->mGuideStr.Empty();

	// ウインドウ位置の復元
	in->mLayout->RestoreWindowPosition(this, false);

	// ホットキー登録
	in->mHotKeyPtr = std::make_unique<AppHotKey>(GetSafeHwnd());
	if (in->mHotKeyPtr->Register() == false) {
		CString msg(_T("ホットキーを登録できませんでした。\n他のアプリケーションで使用されている可能性があります。\n"));
		msg += in->mHotKeyPtr->ToString();

		// この時点ではメインウインドウのサイズが未計算状態なので、ここでメッセージボックスを出すと
		// メインウインドウがおかしなサイズで表示されてしまう。
		// それを回避するため、メッセージボックスを出している間はメインウインドウを隠す
		ShowWindow(SW_HIDE);
		AfxMessageBox(msg);
		ShowWindow(SW_SHOW);
		SPDLOG_ERROR(_T("Failed to restiser app hot key!"));
	}
	in->mMainWindowHotKeyPtr = std::make_unique<MainWindowHotKey>();
	in->mMainWindowHotKeyPtr->Register();
	
	// 設定値の読み込み
	GetCommandRepository()->Load();

	UpdateData(FALSE);

	in->mDropTargetDialog.Register(this);
	in->mDropTargetEdit.Register(&in->mKeywordEdit);

	if (pref->IsHideOnStartup()) {
		// 「起動直後に非表示」の場合はウインドウを隠す
		PostMessage(WM_APP+7, 0, 0);
	}
	else {
		// そうでない場合はアクティブにする
		PostMessage(WM_APP+2, 1, 0);
	}
	ClearContent();

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
	in->mInput.Clear();
	in->mCandidates.Clear();

	// 状態変更を通知
	struct LocalInputStatus : public LauncherInput {
		virtual bool HasKeyword() { return false; }
	} status;
	in->mLayout->UpdateInputStatus(&status, false);

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
	int startPos;
	int endPos;
	in->mKeywordEdit.GetSel(startPos, endPos);
	SPDLOG_DEBUG(_T("endPos:{}"), endPos);

	CString trailing;
	launcherapp::matcher::CommandToken tok(in->mInput.GetKeyword());
	bool hasTrailing = tok.GetTrailingString(endPos, trailing);

	bool withSpace = true;
	in->mInput.SetKeyword(cmd->GetName(), withSpace);
	if (hasTrailing) {
		in->mInput.AddArgument(trailing);
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

	in->mLastInputStr = in->mInput.GetKeyword();

	// 音を鳴らす
	AppSound::Get()->PlayInputSound();

	// キー入力でCtrl-Backspaceを入力したとき、不可視文字(0x7E→Backspace)が入力される
	// (Editコントロールの通常の挙動)
	// このアプリはCtrl-Backspaceで入力文字列を全クリアするが、一方で、上記挙動により
	// 入力文字列をクリアした後、0x7Eが挿入されるという謎挙動になるので、ここで0x7Fを明示的に消している
	if (in->mInput.ReplaceInvisibleChars()) {

		// FIXME: 0x7Eが含まれていたらCtrl-Backspace入力とみなす、という、ここの処理は変なので直したい
		in->mInput.RemoveLastWord();

		// 検索リクエスト
		QueryAsync();

		UpdateData(FALSE);

		// キャレット位置も更新する
		in->mKeywordEdit.SetCaretToEnd();
		return;
	}

	// 検索リクエスト
	QueryAsync();
}

// 入力キーワードで検索をリクエストを出す(完了をまたない)
void LauncherMainWindow::QueryAsync()
{
	PERFLOG("QueryAsync Start");
	spdlog::stopwatch sw;

	// 検索リクエスト
	in->mIsQueryDoing = true;
	auto req = new MainWindowCommandQueryRequest(in->mInput.GetKeyword(), GetSafeHwnd(), WM_APP+13);
	GetCommandRepository()->Query(req);
	req->Release();

	PERFLOG("QueryAsync End {0:.6f} s.", sw);
}


// 入力キーワードで検索をリクエストを出し、完了を待つ
void LauncherMainWindow::QuerySync()
{
	// キーワードによる絞り込みを実施
	in->mIsQueryDoing = true;
	auto req = new MainWindowCommandQueryRequest(in->mInput.GetKeyword(), GetSafeHwnd(), WM_APP+13);
	GetCommandRepository()->Query(req);
	req->Release();

	WaitQueryRequest();
}

// 候補欄を更新
void LauncherMainWindow::UpdateCandidates()
{
	spdlog::debug("LauncherMainWindow::UpdateCandidates start");

	// 入力テキストが空文字列の場合はデフォルト表示に戻す
	if (in->mInput.HasKeyword() == false) {
		ClearContent();
		return;
	}
	// 状態変更を通知
	in->mLayout->UpdateInputStatus(&in->mInput, false);

	auto pCmd = GetCurrentCommand();
	if (pCmd == nullptr) {
		// 候補なし
		CString strMisMatch;
		strMisMatch.LoadString(ID_STRING_MISMATCH);
		in->mGuideStr.Empty();
		SetDescription(strMisMatch);
		in->mIconLabel.DrawIcon(IconLoader::Get()->LoadUnknownIcon());
		in->mCandidateListBox.Invalidate(TRUE);
		return;
	}
	else {
		// サムネイルの更新		
		in->mIconLabel.DrawIcon(pCmd->GetIcon());

		// ガイド欄
		in->mGuideStr = pCmd->GetGuideString();

		// 説明欄の更新
		CString descriptionStr = in->mCandidates.GetCurrentCommandDescription();
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

void
LauncherMainWindow::RunCommand(
	launcherapp::core::Command* cmd,
	launcherapp::core::CommandParameterBuilder* commandParam
)
{
	// コマンド実行後のクローズ方法
	auto closePolicy = launcherapp::core::SelectionBehavior::CLOSEWINDOW_ASYNC;
	spdlog::debug("closePolicy: {}", (int)closePolicy);

	// コマンド実行後のクローズ方法を取得する
	RefPtr<launcherapp::core::SelectionBehavior> behavior;
	if (cmd->QueryInterface(IFID_SELECTIONBEHAVIOR, (void**)&behavior)) {
		closePolicy = behavior->GetCloseWindowPolicy();
	}

	auto hwnd = GetSafeHwnd();

	std::thread th([cmd, commandParam, hwnd, closePolicy]() {

		if (cmd->Execute(commandParam) == FALSE) {
			auto errMsg = cmd->GetErrorString();
			if (errMsg.IsEmpty() == FALSE) {
				auto app = (LauncherApp*)AfxGetApp();
				app->PopupMessage(errMsg);
			}
		}

		commandParam->Release();

		// コマンドの参照カウントを下げる
		cmd->Release();

		if (closePolicy == launcherapp::core::SelectionBehavior::CLOSEWINDOW_SYNC) {
			// コマンドの実行を待ってウインドウを非表示する場合
			::SendMessage(hwnd, WM_APP+18, 0, 0);   // ClearContent
			::SendMessage(hwnd, LauncherMainWindowMessageID::HIDEWINDOW, 0, 0);
		}

	});
	th.detach();

	if (closePolicy == launcherapp::core::SelectionBehavior::CLOSEWINDOW_ASYNC) {
		// コマンドの実行を待たずにウインドウを非表示する場合
		ClearContent();
		HideWindow();
	}
	else if (closePolicy == launcherapp::core::SelectionBehavior::CLOSEWINDOW_NOCLOSE) {
		// ウインドウを閉じない
		GetDlgItem(IDC_EDIT_COMMAND)->SetFocus();
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

	// 別スレッドで処理するのでコピーを生成する
	CString str = in->mInput.GetKeyword();

	// コマンドの参照カウントを上げる(実行完了時に下げる)
	cmd->AddRef();

	auto commandParam = launcherapp::core::CommandParameterBuilder::Create(str);

	// Ctrlキーが押されているかを設定
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
		commandParam->SetNamedParamBool(_T("CtrlKeyPressed"), true);
	}
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
		commandParam->SetNamedParamBool(_T("ShiftKeyPressed"), true);
	}
	if (GetAsyncKeyState(VK_LWIN) & 0x8000) {
		commandParam->SetNamedParamBool(_T("WinKeyPressed"), true);
	}
	if (GetAsyncKeyState(VK_MENU) & 0x8000) {
		commandParam->SetNamedParamBool(_T("AltKeyPressed"), true);
	}

	// 実行
	RunCommand(cmd, commandParam);
}

/**
 	コマンドが持つコンテキストメニューを実行する
 	@param[in] cmd 実行対象のコマンドオブジェクト
 	@param[in] index メニューのインデックス
*/
void LauncherMainWindow::SelectCommandContextMenu(
	launcherapp::core::Command* cmd,
	int index
)
{
	// 実行時の音声を再生する
	//AppSound::Get()->PlayExecuteSound();

	// 別スレッドで処理するのでコピーを生成する
	CString str = in->mInput.GetKeyword();

	// コマンドの参照カウントを上げる(実行完了時に下げる)
	cmd->AddRef();

	std::thread th([cmd, index, str]() {

		RefPtr<launcherapp::commands::core::ContextMenuSource> menuSrc;
		if (cmd->QueryInterface(IFID_CONTEXTMENUSOURCE, (void**)&menuSrc) == false) {
			cmd->Release();
			return;
		}

		auto commandParam = launcherapp::core::CommandParameterBuilder::Create(str);

		if (menuSrc->SelectMenuItem(index, commandParam) == false) {
			auto errMsg = cmd->GetErrorString();
			if (errMsg.IsEmpty() == FALSE) {
				auto app = (LauncherApp*)AfxGetApp();
				app->PopupMessage(errMsg);
			}
		}
		commandParam->Release();
		cmd->Release();
	});
	th.detach();
}


void LauncherMainWindow::OnOK()
{
	UpdateData();

	// バックグラウンドで実行中の問い合わせを待つ
	WaitQueryRequest();

	// 問い合わせの結果として得られた選択中の候補を取得する
	auto cmd = GetCurrentCommand();
	if (cmd) {
		// コマンドを実行する
		RunCommand(cmd);
	}
	else {
		// 空文字状態でEnterキーから実行したときはキーワードマネージャを表示
		if (in->mInput.HasKeyword() == false) {
			ExecuteCommand(_T("manager"));
		}
	}
}

void LauncherMainWindow::OnCancel()
{
	// 入力欄に入力中のテキストがあったらクリア、何もなければメインウインドウを非表示にする
	if (in->mInput.HasKeyword()) {
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
	// メッセージの内容に応じてホットキーハンドラを呼ぶ
	if (core::CommandHotKeyManager::GetInstance()->TryCallLocalHotKeyHander(pMsg)) {
		return TRUE;
	}
	return __super::PreTranslateMessage(pMsg);
}

void LauncherMainWindow::OnShowWindow(BOOL bShow, UINT nStatus)
{
	in->mAppearance->OnShowWindow(bShow, nStatus);

	if (bShow) {
		// 表示するタイミングで入力欄にフォーカスを設定する
		GetDlgItem(IDC_EDIT_COMMAND)->SetFocus();
	}
	else {
		// 「隠れるときに入力文字列を消去しない」設定に応じてテキストを消す
		AppPreference* pref = AppPreference::Get();
		if (pref->IsKeepTextWhenDlgHide() == false) {
			ClearContent();
		}
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
	UNREFERENCED_PARAMETER(lParam);

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

	// 音を鳴らす
	AppSound::Get()->PlaySelectSound();
	// 選択された項目に対応するコマンドを現在のコマンドに変更する
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
	UNREFERENCED_PARAMETER(pNMHDR);

	*pResult = 0;
	// ダブルクリックで確定
	OnOK();
}

void LauncherMainWindow::OnSizing(UINT side, LPRECT rect)
{
	__super::OnSizing(side, rect);

	in->mLayout->RecalcWindowSize(GetSafeHwnd(), &in->mInput, side, rect);
	in->mLayout->RecalcControls(GetSafeHwnd(), &in->mInput);
}

void LauncherMainWindow::OnSize(UINT type, int cx, int cy)
{
	__super::OnSize(type, cx, cy);

	in->mLayout->RecalcControls(GetSafeHwnd(), &in->mInput);

	if (in->mCandidateListBox.GetSafeHwnd()) {
		in->mCandidateListBox.UpdateSize(cx, cy);
	}
}

void LauncherMainWindow::OnMove(int x, int y)
{
	__super::OnMove(x, y);
	in->mLayout->RecalcControls(GetSafeHwnd(), &in->mInput);
}

void LauncherMainWindow::OnMButtonUp(UINT flags, CPoint point)
{
	__super::OnMButtonUp(flags, point);
	in->mKeywordEdit.Paste();
}

/**
 * コンテキストメニューの表示
 */
void LauncherMainWindow::OnContextMenu(
	CWnd* pWnd,
	CPoint point
)
{
	UNREFERENCED_PARAMETER(pWnd);

	SPDLOG_DEBUG(_T("args point:({0},{1})"), point.x, point.y);

	if (point == CPoint(-1, -1)) {
		// リストの要素が選択されていたら、その領域付近にメニューを表示する
		auto listCtrl = GetCandidateList();
		ASSERT(listCtrl);
		CRect rc;
		if (listCtrl->GetCurrentItemRect(&rc)) {
			point.x = rc.left;
			point.y = rc.bottom;
			listCtrl->ClientToScreen(&point);
		}
		else {
			GetCursorPos(&point);
		}
	}

	CMenu menu;
	menu.CreatePopupMenu();

	const UINT ID_COMMAND_TOP = 1;
	const UINT ID_COMMAND_LAST = 1000;
	const UINT ID_SHOW = 1001;
	const UINT ID_HIDE = 1002;
	const UINT ID_NEW = 1003;
	const UINT ID_MANAGER = 1004;
	const UINT ID_APPSETTING = 1005;
	const UINT ID_USERDIR = 1006;
	const UINT ID_RESETPOS = 1007;
	const UINT ID_MANUAL = 1008;
	const UINT ID_VERSIONINFO = 1009;
	const UINT ID_EXIT = 1019;

	BOOL isVisible = IsWindowVisible();
	CString textToggleVisible(isVisible ? (LPCTSTR)IDS_MENUTEXT_HIDE : (LPCTSTR)IDS_MENUTEXT_SHOW);

	// コマンド固有のメニューがあったら取得する
	auto cmd = GetCurrentCommand();
	if (cmd) {
		RefPtr<launcherapp::commands::core::ContextMenuSource> menuSrc;
		if (cmd->QueryInterface(IFID_CONTEXTMENUSOURCE, (void**)&menuSrc)) {
			int count = menuSrc->GetMenuItemCount();
			for (int i = 0; i < count; ++i) {
				LPCTSTR displayName = nullptr;
				menuSrc->GetMenuItemName(i, &displayName);
				if (displayName == nullptr) {
					menu.InsertMenu((UINT)-1, MF_SEPARATOR, 0, _T(""));
					continue;
				}
				menu.InsertMenu((UINT)-1, 0, ID_COMMAND_TOP + i, displayName);
			}
			menu.InsertMenu((UINT)-1, MF_SEPARATOR, 0, _T(""));
		}
	}

	menu.InsertMenu((UINT)-1, 0, isVisible ? ID_HIDE : ID_SHOW, textToggleVisible);
	menu.InsertMenu((UINT)-1, MF_SEPARATOR, 0, _T(""));
	menu.InsertMenu((UINT)-1, 0, ID_APPSETTING, _T("アプリケーションの設定(&S)"));
	menu.InsertMenu((UINT)-1, 0, ID_NEW, _T("新規作成(&N)"));
	menu.InsertMenu((UINT)-1, 0, ID_MANAGER, _T("キーワードマネージャ(&K)"));
	menu.InsertMenu((UINT)-1, MF_SEPARATOR, 0, _T(""));
	menu.InsertMenu((UINT)-1, 0, ID_USERDIR, _T("設定フォルダを開く(&D)"));
	menu.InsertMenu((UINT)-1, 0, ID_RESETPOS, _T("ウインドウ位置をリセット(&R)"));
	menu.InsertMenu((UINT)-1, MF_SEPARATOR, 0, _T(""));
	menu.InsertMenu((UINT)-1, 0, ID_MANUAL, _T("ヘルプ(&H)"));
	menu.InsertMenu((UINT)-1, 0, ID_VERSIONINFO, _T("バージョン情報(&V)"));
	menu.InsertMenu((UINT)-1, MF_SEPARATOR, 0, _T(""));
	menu.InsertMenu((UINT)-1, 0, ID_EXIT, _T("終了"));

	spdlog::info(_T("Show context menu."));

	// TrackPopupMenuをよぶ時点でSetForegroundWindowでウインドウをアクティブにしておかないと
	// ポップアップメニュー以外の領域をクリックしたときにメニューが閉じないので、ここで呼んでおく
	SetForegroundWindow();

	int n = menu.TrackPopupMenu(TPM_RETURNCMD, point.x, point.y, this);
	spdlog::debug(_T("selected menu id:{}"), n);
	if (n == 0) {
		// キャンセル
		return;
	}

	if (n <= ID_COMMAND_LAST) {
		int index = n - ID_COMMAND_TOP;
		SelectCommandContextMenu(cmd, index);
	}
	else if (n == ID_SHOW) {
		ActivateWindow();
		return;
	}
	else if (n == ID_HIDE) {
		// Note: 後方の処理でウインドウを消すのでここでは何もしない
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
		in->mLayout->RestoreWindowPosition(this, true);
		return;
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

	// 選択後はウインドウを隠す
	ClearContent();
	HideWindow();
}

void LauncherMainWindow::OnActivate(UINT nState, CWnd* wnd, BOOL bMinimized)
{
	spdlog::debug("OnActivate nState {}", nState);
	if (in->mAppearance) {
		in->mAppearance->OnActivate(nState, wnd, bMinimized);
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
	// 入力ウインドウのヘルプ表示
	SPDLOG_DEBUG("start");
	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate("InputWindow");
}

void LauncherMainWindow::OnCommandHotKey(UINT id)
{
	// ローカルホットキーに対応されたコマンドを実行する
	SPDLOG_DEBUG("args id:{}", id);
	core::CommandHotKeyManager::GetInstance()->InvokeLocalHandler(id);
}

void LauncherMainWindow::OnTimer(UINT_PTR timerId)
{
	if (timerId == TIMERID_OPERATION) {
		LauncherWindowEventDispatcher::Get()->Dispatch([](LauncherWindowEventListenerIF* listener) {
			listener->OnTimer();
		});
	}
}

LRESULT LauncherMainWindow::OnMessageSessionChange(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

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

HBRUSH LauncherMainWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH defBr = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	return in->mAppearance->OnCtlColor(pDC, pWnd, nCtlColor, defBr);
}

void LauncherMainWindow::OnMeasureItem(int ctrlId, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if (ctrlId == IDC_LIST_CANDIDATE) {
		// 候補欄コントロールクラス側で行の高さを計算し、決定する
		in->mCandidateListBox.OnMeasureItem(lpMeasureItemStruct);
	}
}

