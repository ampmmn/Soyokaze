
// BWLite.cpp : アプリケーションのクラス動作を定義します。
//

#include "pch.h"
#include "framework.h"
#include "BWLite.h"
#include "BWLiteDlg.h"
#include "SharedHwnd.h"
#include "AppProfile.h"
#include <locale.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBWLiteApp

BEGIN_MESSAGE_MAP(CBWLiteApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CBWLiteApp の構築

CBWLiteApp::CBWLiteApp() : m_hMutexRun(NULL)
{
#ifdef UNICODE
	_tsetlocale(LC_ALL, _T(""));
#endif

	// 再起動マネージャーをサポートします
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: この位置に構築用コードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}

CBWLiteApp::~CBWLiteApp()
{
	if (m_hMutexRun != NULL) {
		CloseHandle(m_hMutexRun);
	}
}

// 唯一の CBWLiteApp オブジェクト

CBWLiteApp theApp;


// CBWLiteApp の初期化

BOOL CBWLiteApp::InitInstance()
{
	// 既にプロセスが起動している場合はプロセスをアクティブ化する
	LPCTSTR mutexName = _T("Global\\mutex_BWLite_exist");
	HANDLE h = OpenMutex(MUTEX_ALL_ACCESS, FALSE, mutexName);
	if (h) {
		CloseHandle(h);
		ActivateExistingProcess();
		return FALSE;
 	}

	// 多重起動検知のための名前付きミューテックスを作っておく
	m_hMutexRun = CreateMutex(NULL, FALSE, mutexName);
	if (m_hMutexRun == NULL) {
		if (GetLastError() == ERROR_ACCESS_DENIED) {
			AfxMessageBox(_T("起動に失敗しました。\n管理者権限で既に起動されている可能性があります。"));
			return FALSE;
		}
		AfxMessageBox(_T("Failed to init."));
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

	// 標準初期化
	// これらの機能を使わずに最終的な実行可能ファイルの
	// サイズを縮小したい場合は、以下から不要な初期化
	// ルーチンを削除してください。
	// 設定が格納されているレジストリ キーを変更します。
	SetRegistryKey(_T("YMGW"));

	CBWLiteDlg dlg;
	m_pMainWnd = &dlg;

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
	if (_tcscmp(title, _T("BWLite")) != 0) {
		return TRUE;
	}

	// 手抜きなのでその場でアクティブ化する

	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);

	return FALSE;
}


void CBWLiteApp::ActivateExistingProcess()
{
	SharedHwnd sharedHwnd;
	HWND hwnd = sharedHwnd.GetHwnd();
	if (hwnd == NULL) {
		// 共有メモリ経由で拾えなかった場合は、念のためウインドウを探す
		EnumChildWindows(HWND_DESKTOP, OnEnumChildWindow, (LPARAM)&hwnd);
	}

	if (hwnd == NULL) {
		return;
	}

	if (IsWindowVisible(hwnd) == FALSE) {
		ShowWindow(hwnd, SW_SHOW);
		SetForegroundWindow(hwnd);
	}
	else {
		ShowWindow(hwnd, SW_HIDE);
	}
}
