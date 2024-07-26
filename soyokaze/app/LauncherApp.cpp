
// LauncherApp.cpp : アプリケーションのクラス動作を定義します。
//

#include "pch.h"
#include "framework.h"
#include "app/LauncherApp.h"
#include "mainwindow/LauncherMainWindow.h"
#include "tasktray/TaskTray.h"
#include "setting/AppPreference.h"
#include "app/StartupParam.h"
#include "logger/Logger.h"
#include "SharedHwnd.h"
#include <locale.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static LPCTSTR PROCESS_MUTEX_NAME = _T("Global\\mutex_launcherapp_exist");

// LauncherApp

BEGIN_MESSAGE_MAP(LauncherApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// LauncherApp の構築

LauncherApp::LauncherApp() : m_hMutexRun(NULL)
{
#ifdef UNICODE
	_tsetlocale(LC_ALL, _T(""));
#endif
	// 再起動マネージャーをサポートします
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

LauncherApp::~LauncherApp()
{
	if (m_hMutexRun != NULL) {
		CloseHandle(m_hMutexRun);
	}
}

// 唯一の LauncherApp オブジェクト

LauncherApp theApp;


// LauncherApp の初期化

BOOL LauncherApp::InitInstance()
{
	AppPreference::Get()->Init();

	// ログ初期化
	Logger::Get()->Initialize();
	spdlog::info(_T("==== Start App ===="));

	HRESULT hr = CoInitialize(nullptr);
	if (FAILED(hr)) {
		SPDLOG_ERROR(_T("Failed to CoInitialize!"));
	}

	if (LauncherAppProcessExists() == false) {
		// 通常の起動
		InitFirstInstance();
	}
	else {
		// 既にプロセスが起動している場合は起動しない(先行プロセスを有効化したりなどする)
		InitSecondInstance();
	}

	AppPreference::Get()->OnExit();

	CoUninitialize();

	spdlog::info(_T("==== Exit App ===="));
	return FALSE;
}

/**
 * 既存のランチャーアプリプロセスが存在しない場合の初期化処理
 */
BOOL LauncherApp::InitFirstInstance()
{
	SPDLOG_DEBUG(_T("start"));

	// 多重起動検知のための名前付きミューテックスを作っておく
	m_hMutexRun = CreateMutex(NULL, TRUE, PROCESS_MUTEX_NAME);
	if (m_hMutexRun == NULL) {
		DWORD lastErr = GetLastError(); 
		if (lastErr == ERROR_ACCESS_DENIED) {
			AfxMessageBox(_T("起動に失敗しました。\n管理者権限で既に起動されている可能性があります。"));
		}
		else {
			AfxMessageBox(_T("Failed to init."));
		}
		spdlog::error(_T("Failed to create mutex. err:{0:x}"), lastErr);
		return FALSE;
	}

	if (!AfxOleInit()) {
		AfxMessageBox(_T("Failed to init(AfxOleInit)."));
		spdlog::error(_T("Failed to init(AfxOleInit)."));
		return FALSE;
	}

	// 設定ファイル作成用のフォルダをつくる
	if (AppPreference::Get()->CreateUserDirectory() == false) {
		AfxMessageBox(_T("Warning: Failed to init profile folder."));
		spdlog::error(_T("Warning: Failed to init profile folder."));
	}


	// アプリケーション マニフェストが visual スタイルを有効にするために、
	// ComCtl32.dll Version 6 以降の使用を指定する場合は、
	// Windows XP に InitCommonControlsEx() が必要です。さもなければ、ウィンドウ作成はすべて失敗します。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// アプリケーションで使用するすべてのコモン コントロール クラスを含めるには、
	// これを設定します。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// ダイアログにシェル ツリー ビューまたはシェル リスト ビュー コントロールが
	// 含まれている場合にシェル マネージャーを作成します。
	CShellManager pShellManager;

	// MFC コントロールでテーマを有効にするために、"Windows ネイティブ" のビジュアル マネージャーをアクティブ化
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	LauncherMainWindow dlg;
	m_pMainWnd = &dlg;

	try {
		mTaskTray.reset(new TaskTray(&dlg));
		mTaskTray->Create();

		dlg.DoModal();
	}
	catch(...) {
		mTaskTray.reset();
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	return FALSE;
}

/**
 * 既存のランチャーアプリプロセスが存在する場合の初期化処理
 */
BOOL LauncherApp::InitSecondInstance()
{
	SPDLOG_DEBUG(_T("start"));

	StartupParam startupParam(__argc, __targv);

	bool hasRunCommand = false;
	CString value;
	while (startupParam.HasRunCommand(value)) {
		// -cオプションでコマンドが与えられた場合、既存プロセス側にコマンドを送り、終了する
		SendCommandString(value, false);
		hasRunCommand = true;
		startupParam.ShiftRunCommand();
	}

	if (hasRunCommand) {
		// -cオプションを処理したら、プログラムを終了する
		return FALSE;
	}

	if (startupParam.HasPathToRegister(value)) {
		// 第一引数で存在するパスが指定された場合は登録画面を表示する
		RegisterPath(value);
		return FALSE;
	}

	if (startupParam.HasHideOption()) {
		SharedHwnd sharedHwnd;
		HWND hwnd = sharedHwnd.GetHwnd();
		if (IsWindow(hwnd)) {
			PostMessage(hwnd, WM_APP+7, 0, 0);
		}
		return FALSE;
	}

	// プロセスをアクティブ化し、このプロセスは終了する
	ActivateExistingProcess();

	if (startupParam.HasPasteOption(value)) {
		// /pasteオプションでコマンドが与えられた場合、既存プロセス側にテキストを送る
		// 直前で実行したActivateExistingProcessにより、入力欄のクリアが走るため、テキストを送る処理をあとに行っている
		spdlog::debug(_T("HasPaste value:{}"), (LPCTSTR)value);

		bool isPasteOnly = true;
		SendCommandString(value, isPasteOnly);
	}

	// 選択範囲を指定するオプションが指定されていたら範囲を送信する
	int startPos = -1, selLength = 0;
	if (startupParam.GetSelectRange(startPos, selLength)) {
		SendCaretRange(startPos, selLength);
	}

	return FALSE;
}

/**
 * 先行するアプリのプロセスが存在するか?
 * @return true: 存在する  false: 存在しない
 */
bool LauncherApp::LauncherAppProcessExists()
{
	HANDLE h = OpenMutex(MUTEX_ALL_ACCESS, FALSE, PROCESS_MUTEX_NAME);
	bool isExists = (h != nullptr);
	if (h != nullptr) {
		CloseHandle(h);
	}

	SPDLOG_DEBUG(_T("isExists={}"), isExists);
	return isExists;
}

/**
 * @return true: アクティブ化した  false: 先行プロセスはない
 */
bool LauncherApp::ActivateExistingProcess()
{
	SPDLOG_DEBUG(_T("start"));

	// 先行プロセスを有効化する
	SharedHwnd sharedHwnd;
	HWND hwnd = sharedHwnd.GetHwnd();
	if (hwnd == NULL) {
		return false;
	}

	LauncherMainWindow::ActivateWindow(hwnd);

	return true;
}

/**
 *  先行プロセスに対しコマンド文字列を送る
 *  (先行プロセス側でコマンドを実行する)
 */
bool LauncherApp::SendCommandString(const CString& commandStr, bool isPasteOnly)
{
	SPDLOG_DEBUG(_T("args commandStr:{0}"), (LPCTSTR)commandStr);

	SharedHwnd sharedHwnd;
	HWND hwnd = sharedHwnd.GetHwnd();
	if (hwnd == NULL) {
		return false;
	}

	HWND hwndCommand = GetDlgItem(hwnd, IDC_EDIT_COMMAND2);
	if (hwndCommand == NULL) {
		SPDLOG_ERROR(_T("CmdReceiveEdit does not found."));
		return false;
	}

	if (isPasteOnly) {
		SendMessage(hwndCommand, WM_APP+1, 0, 0);
	}
	SendMessage(hwndCommand, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)commandStr);
	return true;
}

bool LauncherApp::SendCaretRange(int startPos, int length)
{
	SPDLOG_DEBUG(_T("args startPos:{0} length:{1}"), startPos, length);

	SharedHwnd sharedHwnd;
	HWND hwnd = sharedHwnd.GetHwnd();
	if (hwnd == NULL) {
		return false;
	}

	HWND hwndCommand = GetDlgItem(hwnd, IDC_EDIT_COMMAND2);
	if (hwndCommand == NULL) {
		SPDLOG_ERROR(_T("CmdReceiveEdit does not found."));
		return false;
	}

	SendMessage(hwndCommand, WM_APP+2, startPos, length);
	return true;
}

/**
 *  指定されたパスをコマンドとして登録する
 *  @return true: 成功 false:失敗
 *  @param pathStr  登録対象のファイルパス
 */
bool LauncherApp::RegisterPath(const CString& pathStr)
{
	SPDLOG_DEBUG(_T("args path:{0}"), (LPCTSTR)pathStr);

	CString name = PathFindFileName(pathStr);
	PathRemoveExtension(name.GetBuffer(name.GetLength()));
	name.ReleaseBuffer();

	// 空白を置換
	name.Replace(_T(' '), _T('_'));

	CString commandStr(_T("new "));
	commandStr += _T("\"") + name + _T("\" \"") + pathStr + _T("\"");
	return SendCommandString(commandStr, false);
}

// バルーンメッセージを表示
bool LauncherApp::PopupMessage(const CString& message)
{
	SPDLOG_DEBUG(_T("args msg:{0}"), (LPCTSTR)message);
	if (!mTaskTray) {
		return false;
	}

	mTaskTray->ShowMessage(message);
	return true;
}

