#include "pch.h"
#include "framework.h"
#include "ShortcutSettingPage.h"
#include "utility/ShortcutFile.h"
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
		link.SetLinkPath(mSoyokazePath);
		link.Save(mSendToPath);
	}

	if (mStartMenu == FALSE && PathFileExists(mStartMenuPath)) {
		DeleteFile(mStartMenuPath);
		RemoveDirectory(mStartMenuDir);
	}
	else if (mStartMenu && PathFileExists(mStartMenuPath) == FALSE) {
		CreateDirectory(mStartMenuDir, NULL);
		ShortcutFile link;
		link.SetLinkPath(mSoyokazePath);
		link.Save(mStartMenuPath);
	}

	if (mDesktop == FALSE && PathFileExists(mDesktopPath)) {
		DeleteFile(mDesktopPath);
	}
	else if (mDesktop && PathFileExists(mDesktopPath) == FALSE) {
		ShortcutFile link;
		link.SetLinkPath(mSoyokazePath);
		link.Save(mDesktopPath);
	}

	if (mStartup == FALSE && PathFileExists(mStartupPath)) {
		DeleteFile(mStartupPath);
	}
	else if (mStartup && PathFileExists(mStartupPath) == FALSE) {
		ShortcutFile link;
		link.SetLinkPath(mSoyokazePath);
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

static void MakeShortcutPath(CString& out, int type, LPCTSTR linkName)
{
	LPTSTR path = out.GetBuffer(MAX_PATH_NTFS);
	SHGetSpecialFolderPath(NULL, path, type, 0);
	PathAppend(path, linkName);
	out.ReleaseBuffer();
}


BOOL ShortcutSettingPage::OnInitDialog()
{
	__super::OnInitDialog();

	GetModuleFileName(NULL, mSoyokazePath.GetBuffer(MAX_PATH_NTFS), MAX_PATH_NTFS);
	mSoyokazePath.ReleaseBuffer();

	LPCTSTR exeName = PathFindFileName(mSoyokazePath);

	TCHAR linkName[MAX_PATH_NTFS];
	_tcscpy_s(linkName, exeName);
	PathRenameExtension(linkName, _T(".lnk"));

	CString linkNameForSendTo((LPCTSTR)IDS_SENDTO);

	TCHAR linkNameForStartMenu[MAX_PATH_NTFS];
	_tcscpy_s(linkNameForStartMenu, exeName);
	PathRemoveExtension(linkNameForStartMenu);
	PathAppend(linkNameForStartMenu, exeName);
	PathRenameExtension(linkNameForStartMenu, _T(".lnk"));

	MakeShortcutPath(mSendToPath, CSIDL_SENDTO, linkNameForSendTo);
	MakeShortcutPath(mStartMenuPath, CSIDL_PROGRAMS, linkNameForStartMenu);

	mStartMenuDir = mStartMenuPath;
	PathRemoveFileSpec(mStartMenuDir.GetBuffer(MAX_PATH_NTFS));
	mStartMenuDir.ReleaseBuffer();


	MakeShortcutPath(mDesktopPath, CSIDL_DESKTOP, linkName);
	MakeShortcutPath(mStartupPath, CSIDL_STARTUP, linkName);

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
	LPCTSTR exeName = PathFindFileName(mSoyokazePath);

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
