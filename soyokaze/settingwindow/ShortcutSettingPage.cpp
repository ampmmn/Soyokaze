#include "pch.h"
#include "framework.h"
#include "ShortcutSettingPage.h"
#include "utility/ShortcutFile.h"
#include "app/AppName.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


ShortcutSettingPage::ShortcutSettingPage(CWnd* parentWnd) : 
	SettingPage(_T("ショートカット登録"), IDD_SHORTCUTSETTING, parentWnd)
{
}

ShortcutSettingPage::~ShortcutSettingPage()
{
}

void ShortcutSettingPage::OnOK()
{
	if (mSendTo == FALSE && PathFileExists(mSendToPath)) {
		DeleteFile(mSendToPath);
	}
	else if (mSendTo && PathFileExists(mSendToPath) == FALSE) {
		ShortcutFile link;
		link.SetLinkPath(mAppPath);
		link.Save(mSendToPath);
	}

	if (mStartMenu == FALSE && PathFileExists(mStartMenuPath)) {
		DeleteFile(mStartMenuPath);
		RemoveDirectory(mStartMenuDir);
	}
	else if (mStartMenu && PathFileExists(mStartMenuPath) == FALSE) {
		CreateStartMenu();
	}

	if (mDesktop == FALSE && PathFileExists(mDesktopPath)) {
		DeleteFile(mDesktopPath);
	}
	else if (mDesktop && PathFileExists(mDesktopPath) == FALSE) {
		ShortcutFile link;
		link.SetLinkPath(mAppPath);
		link.Save(mDesktopPath);
	}

	if (mStartup == FALSE && PathFileExists(mStartupPath)) {
		DeleteFile(mStartupPath);
	}
	else if (mStartup && PathFileExists(mStartupPath) == FALSE) {
		ShortcutFile link;
		link.SetLinkPath(mAppPath);
		link.Save(mStartupPath);
	}

	__super::OnOK();
}

void ShortcutSettingPage::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_SENDTO, mSendTo);
	DDX_Check(pDX, IDC_CHECK_STARTMENU, mStartMenu);
	DDX_Check(pDX, IDC_CHECK_DESKTOP, mDesktop);
	DDX_Check(pDX, IDC_CHECK_STARTUP, mStartup);
}

BEGIN_MESSAGE_MAP(ShortcutSettingPage, SettingPage)
	ON_COMMAND(IDC_BUTTON_DELETE, OnButtonDelete)
END_MESSAGE_MAP()

BOOL ShortcutSettingPage::OnInitDialog()
{
	__super::OnInitDialog();

	GetModuleFileName(NULL, mAppPath.GetBuffer(MAX_PATH_NTFS), MAX_PATH_NTFS);
	mAppPath.ReleaseBuffer();

	LPCTSTR exeName = PathFindFileName(mAppPath);

	TCHAR linkName[MAX_PATH_NTFS];
	_tcscpy_s(linkName, exeName);
	PathRenameExtension(linkName, _T(".lnk"));

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

BOOL ShortcutSettingPage::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL ShortcutSettingPage::OnSetActive()
{
	return TRUE;
}

void ShortcutSettingPage::UpdateStatus()
{
	mSendTo = PathFileExists(mSendToPath);
	mStartMenu = PathFileExists(mStartMenuPath);
	mDesktop = PathFileExists(mDesktopPath);
	mStartup = PathFileExists(mStartupPath);
	UpdateData(FALSE);
}

void ShortcutSettingPage::OnButtonDelete()
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

void ShortcutSettingPage::OnEnterSettings()
{
}

// スタートメニューのパス生成
void ShortcutSettingPage::CreateStartMenuPath(CString& pathToMenu)
{
	TCHAR appPath[MAX_PATH_NTFS];
	GetModuleFileName(NULL, appPath, MAX_PATH_NTFS);

	LPCTSTR exeName = PathFindFileName(appPath);

	TCHAR linkNameForStartMenu[MAX_PATH_NTFS];
	_tcscpy_s(linkNameForStartMenu, exeName);
	PathRemoveExtension(linkNameForStartMenu);
	// フォルダ作成
	PathAppend(linkNameForStartMenu, exeName);
	PathRenameExtension(linkNameForStartMenu, _T(".lnk"));
	// ショートカットファイルのパス作成
	ShortcutFile::MakeSpecialFolderPath(pathToMenu, CSIDL_PROGRAMS, linkNameForStartMenu);
}

// スタートメニューは登録済か?
bool ShortcutSettingPage::IsStartMenuExists()
{
	CString path;
	CreateStartMenuPath(path);
	return PathFileExists(path) != FALSE;
}

bool ShortcutSettingPage::CreateStartMenu()
{
	CString pathToStartMenu;
	CreateStartMenuPath(pathToStartMenu);

	CString pathToDir(pathToStartMenu);
	PathRemoveFileSpec(pathToDir.GetBuffer(MAX_PATH_NTFS));
	pathToDir.ReleaseBuffer();

	TCHAR appPath[MAX_PATH_NTFS];
	GetModuleFileName(NULL, appPath, MAX_PATH_NTFS);
	CreateDirectory(pathToDir, NULL);

	ShortcutFile link;

	link.SetLinkPath(appPath);
	link.SetAppId(LAUNCHER_APPID);
	link.SetToastCallbackGUID(LAUNCHER_TOAST_CALLBACK_GUID);
	link.Save(pathToStartMenu);

	return true;
}

bool ShortcutSettingPage::GetHelpPageId(CString& id)
{
	id = _T("ShortcutSetting");
	return true;
}

