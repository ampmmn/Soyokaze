#include "pch.h"
#include "framework.h"
#include "ShortcutSettingPage.h"
#include "setting/Settings.h"
#include "utility/ShortcutFile.h"
#include "utility/Path.h"
#include "app/AppName.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class AppSettingPageShortcut::SettingPage : public CDialog
{
public:
	static void CreateStartMenuPath(CString& pathToMenu);
	static bool CreateStartMenu();

public:
	CString mAppPath;

	// 各種ショートカットのパス
	CString mSendToPath;
	CString mStartMenuDir;
	CString mStartMenuPath;
	CString mDesktopPath;
	CString mStartupPath;

	BOOL mSendTo{FALSE};
	BOOL mStartMenu{FALSE};
	BOOL mDesktop{FALSE};
	BOOL mStartup{FALSE};


	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

	void UpdateStatus();

	bool MakeShortcutSendToPath();
	bool MakeShortcutStartMenu();
	bool MakeShortcutDesktop();
	bool MakeShortcutStartup();
// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonDelete();
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct AppSettingPageShortcut::PImpl
{
	SettingPage mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageShortcut)

AppSettingPageShortcut::AppSettingPageShortcut() : 
	AppSettingPageBase(_T("基本"), _T("ショートカット登録")),
	in(new PImpl)
{
}

AppSettingPageShortcut::~AppSettingPageShortcut()
{
}

// スタートメニューは登録済か?
bool AppSettingPageShortcut::IsStartMenuExists()
{
	CString path;
	SettingPage::CreateStartMenuPath(path);
	return Path::FileExists(path) != FALSE;
}


// ウインドウを作成する
bool AppSettingPageShortcut::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_SHORTCUTSETTING, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageShortcut::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageShortcut::GetOrder()
{
	return 10;
}
// 
bool AppSettingPageShortcut::OnEnterSettings()
{
	// このページは設定への読み書きをしない
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageShortcut::OnSetActive()
{
	return true;
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageShortcut::OnKillActive()
{
	return true;
}
//
void AppSettingPageShortcut::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageShortcut::GetHelpPageId(String& id)
{
	id = "ShortcutSetting";
	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void AppSettingPageShortcut::SettingPage::OnOK()
{
	MakeShortcutSendToPath();
	MakeShortcutStartMenu();
	MakeShortcutDesktop();
	MakeShortcutStartup();

	__super::OnOK();
}

bool AppSettingPageShortcut::SettingPage::MakeShortcutSendToPath()
{
	if (mSendTo == FALSE && Path::FileExists(mSendToPath)) {
		if (DeleteFile(mSendToPath) == FALSE) {
			spdlog::warn(_T("Failed to delete send to path shortcut."));
		}
		return true;
	}
	if (mSendTo && Path::FileExists(mSendToPath) == FALSE) {
		ShortcutFile link;
		link.SetLinkPath(mAppPath);
		link.Save(mSendToPath);
	}
	return true;
}

bool AppSettingPageShortcut::SettingPage::MakeShortcutStartMenu()
{
	if (mStartMenu == FALSE && Path::FileExists(mStartMenuPath)) {
		// スタートメニューを作成しない、かつ、存在する場合は消す
		if (DeleteFile(mStartMenuPath) == FALSE ||
		    RemoveDirectory(mStartMenuDir) == FALSE) {
			spdlog::warn(_T("Failed to delete start menu."));
		}
		return true;
	}

	if (mStartMenu && Path::FileExists(mStartMenuPath) == FALSE) {
		// スタートメニューを作成する、かつ、存在しない場合は作成する
		CreateStartMenu();
	}
	return true;
}

bool AppSettingPageShortcut::SettingPage::MakeShortcutDesktop()
{
	if (mDesktop == FALSE && Path::FileExists(mDesktopPath)) {
		// デスクトップにアイコンを作成しない、かつ、存在する場合は消す
		if (DeleteFile(mDesktopPath) == FALSE) {
			spdlog::warn(_T("Failed to delete desktop shortcut."));
		}
		return true;
	}

	if (mDesktop && Path::FileExists(mDesktopPath) == FALSE) {
		// デスクトップにアイコンを作成する、かつ、存在しない場合は作成する
		ShortcutFile link;
		link.SetLinkPath(mAppPath);
		link.Save(mDesktopPath);
	}
	return true;
}

bool AppSettingPageShortcut::SettingPage::MakeShortcutStartup()
{
	if (mStartup == FALSE && Path::FileExists(mStartupPath)) {
		if (DeleteFile(mStartupPath) == FALSE) {
			spdlog::warn(_T("Failed to delete startup shortcut."));
		}
		return true;
	}
	if (mStartup && Path::FileExists(mStartupPath) == FALSE) {
		ShortcutFile link;
		link.SetLinkPath(mAppPath);
		link.Save(mStartupPath);
	}
	return true;
}


void AppSettingPageShortcut::SettingPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_SENDTO, mSendTo);
	DDX_Check(pDX, IDC_CHECK_STARTMENU, mStartMenu);
	DDX_Check(pDX, IDC_CHECK_DESKTOP, mDesktop);
	DDX_Check(pDX, IDC_CHECK_STARTUP, mStartup);
}

BEGIN_MESSAGE_MAP(AppSettingPageShortcut::SettingPage, CDialog)
	ON_COMMAND(IDC_BUTTON_DELETE, OnButtonDelete)
END_MESSAGE_MAP()

BOOL AppSettingPageShortcut::SettingPage::OnInitDialog()
{
	__super::OnInitDialog();

	GetModuleFileName(NULL, mAppPath.GetBuffer(MAX_PATH_NTFS), MAX_PATH_NTFS);
	mAppPath.ReleaseBuffer();

	LPCTSTR exeName = PathFindFileName(mAppPath);

	Path linkName(exeName);
	linkName.RenameExtension(_T(".lnk"));

	// 「送る」のパス生成
	CString linkNameForSendTo((LPCTSTR)IDS_SENDTO);
	ShortcutFile::MakeSpecialFolderPath(mSendToPath, CSIDL_SENDTO, linkNameForSendTo);

	// スタートメニューのパス生成
	CreateStartMenuPath(mStartMenuPath);
	mStartMenuDir = mStartMenuPath;
	PathRemoveFileSpec(mStartMenuDir.GetBuffer(MAX_PATH_NTFS));
	mStartMenuDir.ReleaseBuffer();


	ShortcutFile::MakeSpecialFolderPath(mDesktopPath, CSIDL_DESKTOP, linkName);
	ShortcutFile::MakeSpecialFolderPath(mStartupPath, CSIDL_STARTUP, linkName);

	UpdateStatus();

	return TRUE;
}


void AppSettingPageShortcut::SettingPage::UpdateStatus()
{
	mSendTo = Path::FileExists(mSendToPath);
	mStartMenu = Path::FileExists(mStartMenuPath);
	mDesktop = Path::FileExists(mDesktopPath);
	mStartup = Path::FileExists(mStartupPath);
	UpdateData(FALSE);
}

void AppSettingPageShortcut::SettingPage::OnButtonDelete()
{
	LPCTSTR exeName = PathFindFileName(mAppPath);

	CString msg((LPCTSTR)IDS_CONFIRM_DELETESHORTCUT);
	msg.Replace(_T("$APPNAME"), exeName);
	msg += _T("\n\n");
	msg += _T("※この変更は今すぐ反映されます(キャンセルできません)");

	int n = AfxMessageBox(msg,
	                      MB_YESNO | MB_ICONQUESTION| MB_DEFBUTTON2);
	if (n != IDYES) {
		return;
	}

	// すべてのショートカットを削除
	DeleteFile(mSendToPath);
	DeleteFile(mStartMenuPath);
	RemoveDirectory(mStartMenuDir);
	DeleteFile(mDesktopPath);
	DeleteFile(mStartupPath);

	UpdateStatus();
}

// スタートメニューのパス生成
void AppSettingPageShortcut::SettingPage::CreateStartMenuPath(CString& pathToMenu)
{
	Path appPath(Path::MODULEFILEPATH);

	LPCTSTR exeName = appPath.FindFileName();

	Path linkNameForStartMenu(exeName);
	linkNameForStartMenu.RemoveExtension();
	// フォルダ作成
	linkNameForStartMenu.Append(exeName);
	linkNameForStartMenu.RenameExtension(_T(".lnk"));
	// ショートカットファイルのパス作成
	ShortcutFile::MakeSpecialFolderPath(pathToMenu, CSIDL_PROGRAMS, linkNameForStartMenu);
}

bool AppSettingPageShortcut::SettingPage::CreateStartMenu()
{
	CString pathToStartMenu;
	CreateStartMenuPath(pathToStartMenu);

	Path pathToDir((LPCTSTR)pathToStartMenu);
	pathToDir.RemoveFileSpec();
	if (CreateDirectory(pathToDir, NULL) == FALSE) {
		spdlog::warn(_T("Failed to create start menu folder."));
		return false;
	}

	ShortcutFile link;

	Path appPath(Path::MODULEFILEPATH);
	link.SetLinkPath(appPath);
	link.SetAppId(LAUNCHER_APPID);
	link.SetToastCallbackGUID(LAUNCHER_TOAST_CALLBACK_GUID);
	link.Save(pathToStartMenu);

	return true;
}

