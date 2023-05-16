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
	UpdateData();

	if (mSendTo == FALSE && PathFileExists(mSendToPath)) {
		DeleteFile(mSendToPath);
	}
	else if (mSendTo && PathFileExists(mSendToPath) == FALSE) {
		ShortcutFile link;
		link.SetLinkPath(mBWLitePath);
		link.Save(mSendToPath);
	}

	if (mStartMenu == FALSE && PathFileExists(mStartMenuPath)) {
		DeleteFile(mStartMenuPath);
		RemoveDirectory(mStartMenuDir);
	}
	else if (mStartMenu && PathFileExists(mStartMenuPath) == FALSE) {
		CreateDirectory(mStartMenuDir, NULL);
		ShortcutFile link;
		link.SetLinkPath(mBWLitePath);
		link.Save(mStartMenuPath);
	}

	if (mDesktop == FALSE && PathFileExists(mDesktopPath)) {
		DeleteFile(mDesktopPath);
	}
	else if (mDesktop && PathFileExists(mDesktopPath) == FALSE) {
		ShortcutFile link;
		link.SetLinkPath(mBWLitePath);
		link.Save(mDesktopPath);
	}

	if (mStartup == FALSE && PathFileExists(mStartupPath)) {
		DeleteFile(mStartupPath);
	}
	else if (mStartup && PathFileExists(mStartupPath) == FALSE) {
		ShortcutFile link;
		link.SetLinkPath(mBWLitePath);
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
	LPTSTR path = out.GetBuffer(32768);
	SHGetSpecialFolderPath(NULL, path, type, 0);
	PathAppend(path, linkName);
	out.ReleaseBuffer();
}


BOOL ShortcutSettingPage::OnInitDialog()
{
	__super::OnInitDialog();

	GetModuleFileName(NULL, mBWLitePath.GetBuffer(32768), 32768);
	mBWLitePath.ReleaseBuffer();

	LPCTSTR exeName = PathFindFileName(mBWLitePath);

	TCHAR linkName[32768];
	_tcscpy_s(linkName, exeName);
	PathRenameExtension(linkName, _T(".lnk"));

	TCHAR linkNameForSendTo[32768];
	_tcscpy_s(linkNameForSendTo, exeName);
	PathRemoveExtension(linkNameForSendTo);
	_tcscat_s(linkNameForSendTo, _T("に登録"));
	PathAddExtension(linkNameForSendTo, _T(".lnk"));

	TCHAR linkNameForStartMenu[32768];
	_tcscpy_s(linkNameForStartMenu, exeName);
	PathRemoveExtension(linkNameForStartMenu);
	PathAppend(linkNameForStartMenu, exeName);
	PathRenameExtension(linkNameForStartMenu, _T(".lnk"));

	MakeShortcutPath(mSendToPath, CSIDL_SENDTO, linkNameForSendTo);
	MakeShortcutPath(mStartMenuPath, CSIDL_PROGRAMS, linkNameForStartMenu);

	mStartMenuDir = mStartMenuPath;
	PathRemoveFileSpec(mStartMenuDir.GetBuffer(32768));
	mStartMenuDir.ReleaseBuffer();


	MakeShortcutPath(mDesktopPath, CSIDL_DESKTOP, linkName);
	MakeShortcutPath(mStartupPath, CSIDL_STARTUP, linkName);

	UpdateStatus();

	return TRUE;
}

BOOL ShortcutSettingPage::OnKillActive()
{
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
	LPCTSTR exeName = PathFindFileName(mBWLitePath);

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
