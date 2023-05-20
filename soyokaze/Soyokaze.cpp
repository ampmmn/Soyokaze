
// Soyokaze.cpp : アプリケーションのクラス動作を定義します。
//

#include "pch.h"
#include "framework.h"
#include "Soyokaze.h"
#include "gui/SoyokazeDlg.h"
#include "gui/TaskTray.h"
#include "utility/AppProfile.h"
#include "Arguments.h"
#include "SharedHwnd.h"
#include <locale.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static LPCTSTR PROCESS_MUTEX_NAME = _T("Global\\mutex_Soyokaze_exist");

// CSoyokazeApp

BEGIN_MESSAGE_MAP(CSoyokazeApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CSoyokazeApp の構築

CSoyokazeApp::CSoyokazeApp() : m_hMutexRun(NULL)
{
#ifdef UNICODE
	_tsetlocale(LC_ALL, _T(""));
#endif
	CoInitialize(NULL);

	// 再起動マネージャーをサポートします
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

CSoyokazeApp::~CSoyokazeApp()
{
	if (m_hMutexRun != NULL) {
		CloseHandle(m_hMutexRun);
	}
	CoUninitialize();
}

// 唯一の CSoyokazeApp オブジェクト

CSoyokazeApp theApp;


// CSoyokazeApp の初期化

BOOL CSoyokazeApp::InitInstance()
{
	Arguments args(__argc, __targv);

	// 既にプロセスが起動している場合は起動しない
	if (SoyokazeProcessExists()) {

		CString value;
		if (args.GetBWOptValue(_T("/Runcommand="), value) || args.GetValue(_T("-c"), value)) {
			// -cオプションでコマンドが与えられた場合、既存プロセス側にコマンドを送り、終了する
			SendCommandString(value);
			return FALSE;
		}
		if (args.GetCount() > 1 && PathFileExists(args.Get(1))) {
			// 第一引数で存在するパスが指定された場合は登録画面を表示する
			RegisterPath(args.Get(1));
			return FALSE;
		}
		else {
			// プロセスをアクティブ化し、このプロセスは終了する
			ActivateExistingProcess();
		}
		return FALSE;
	}

	// 多重起動検知のための名前付きミューテックスを作っておく
	m_hMutexRun = CreateMutex(NULL, FALSE, PROCESS_MUTEX_NAME);
	if (m_hMutexRun == NULL) {
		if (GetLastError() == ERROR_ACCESS_DENIED) {
			AfxMessageBox(_T("起動に失敗しました。\n管理者権限で既に起動されている可能性があります。"));
			return FALSE;
		}
		AfxMessageBox(_T("Failed to init."));
		return FALSE;
	}

	if (!AfxOleInit()) {
		AfxMessageBox(_T("Failed to init(AfxOleInit)."));
		return FALSE;
	}

	// 設定ファイル作成用のフォルダをつくる
	if (CAppProfile::CreateProfileDirectory() == false) {
		AfxMessageBox(_T("Warning: Failed to init profile folder."));
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

	CSoyokazeDlg dlg;
	m_pMainWnd = &dlg;

	TaskTray taskTray(&dlg);
	taskTray.Create();

	dlg.DoModal();

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif
	return FALSE;
}

static BOOL CALLBACK OnEnumChildWindow(
	HWND hwnd,
	LPARAM lp
)
{
	lp;

	// Hint表示対象ウインドウかどうかを判定
	TCHAR title[256] = {};
	GetWindowText(hwnd, title, 256);

	// とりあえずウインドウタイトルだけで探す(手抜き)
	if (_tcscmp(title, _T("Soyokaze")) != 0) {
		return TRUE;
	}

	// 手抜きなのでその場でアクティブ化する

	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);

	return FALSE;
}

/**
 * 先行するSoyokazeプロセスが存在するか?
 * @return true: 存在する  false: 存在しない
 */
bool CSoyokazeApp::SoyokazeProcessExists()
{
	HANDLE h = OpenMutex(MUTEX_ALL_ACCESS, FALSE, PROCESS_MUTEX_NAME);
	if (h == NULL) {
		return false;
	}
	CloseHandle(h);
	return true;
}

/**
 * @return true: アクティブ化した  false: 先行プロセスはない
 */
bool CSoyokazeApp::ActivateExistingProcess()
{
	// 先行プロセスを有効化する
	SharedHwnd sharedHwnd;
	HWND hwnd = sharedHwnd.GetHwnd();
	if (hwnd == NULL) {
		// 共有メモリ経由で拾えなかった場合は、念のためウインドウを探す
		EnumChildWindows(HWND_DESKTOP, OnEnumChildWindow, (LPARAM)&hwnd);
	}

	if (hwnd == NULL) {
		return false;
	}

	CSoyokazeDlg::ActivateWindow(hwnd);

	return true;
}

/**
 *  先行プロセスに対しコマンド文字列を送る
 *  (先行プロセス側でコマンドを実行する)
 */
bool CSoyokazeApp::SendCommandString(const CString& commandStr)
{
	SharedHwnd sharedHwnd;
	HWND hwnd = sharedHwnd.GetHwnd();
	if (hwnd == NULL) {
		// 共有メモリ経由で拾えなかった場合は、念のためウインドウを探す
		EnumChildWindows(HWND_DESKTOP, OnEnumChildWindow, (LPARAM)&hwnd);
	}

	if (hwnd == NULL) {
		return false;
	}

	HWND hwndCommand = GetDlgItem(hwnd, IDC_EDIT_COMMAND2);
	if (hwndCommand == NULL) {
		return false;
	}

	SendMessage(hwndCommand, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)commandStr);
	return true;
}

bool CSoyokazeApp::RegisterPath(const CString& pathStr)
{
	CString name = PathFindFileName(pathStr);
	PathRemoveExtension(name.GetBuffer(name.GetLength()));
	name.ReleaseBuffer();

	CString commandStr(_T("new "));
	commandStr += name + _T(" \"") + pathStr + _T("\"");
	return SendCommandString(commandStr);
}

