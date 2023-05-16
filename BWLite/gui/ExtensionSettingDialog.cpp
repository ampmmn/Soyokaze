#include "pch.h"
#include "framework.h"
#include "ExtensionSettingDialog.h"
#include "gui/ShortcutDialog.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


ExtensionSettingDialog::ExtensionSettingDialog(CWnd* parentWnd) : 
	SettingPage(_T("拡張機能"), IDD_EXTENSIONSETTING, parentWnd)
{
}

ExtensionSettingDialog::~ExtensionSettingDialog()
{
}

void ExtensionSettingDialog::OnOK()
{
	__super::OnOK();
}

void ExtensionSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ExtensionSettingDialog, SettingPage)
END_MESSAGE_MAP()


BOOL ExtensionSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	return TRUE;
}

BOOL ExtensionSettingDialog::OnKillActive()
{
	return TRUE;
}

BOOL ExtensionSettingDialog::OnSetActive()
{
	return TRUE;
}
