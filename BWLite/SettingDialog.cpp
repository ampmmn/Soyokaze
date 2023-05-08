#include "pch.h"
#include "framework.h"
#include "SettingDialog.h"
#include "ShortcutDialog.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


SettingDialog::SettingDialog() :  CDialogEx(IDD_SETTING)
{
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_HOTKEY, mHotKey);
	DDX_CBIndex(pDX, IDC_COMBO_MATCHLEVEL, mMatchLevel);
	DDX_Check(pDX, IDC_CHECK_USEFILER, mIsUseExternalFiler);
	DDX_Text(pDX, IDC_EDIT_FILERPATH, mFilerPath);
	DDX_Text(pDX, IDC_EDIT_FILERPARAM, mFilerParam);
	DDX_Check(pDX, IDC_CHECK_TOPMOST, mIsTopMost);
	DDX_CBIndex(pDX, IDC_COMBO_TRANSPARENCY, mTransparencyType);
	DDX_Text(pDX, IDC_EDIT_ALPHA, mAlpha);
	DDV_MinMaxInt(pDX, mAlpha, 0, 255);
}

BEGIN_MESSAGE_MAP(SettingDialog, CDialogEx)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE, OnButtonBrowseFile)
	ON_COMMAND(IDC_BUTTON_SHORTCUT, OnButtonShortcut)
	ON_COMMAND(IDC_CHECK_USEFILER, OnCheckUseFilter)
	ON_CBN_SELCHANGE(IDC_COMBO_TRANSPARENCY, OnCbnTransparencyChanged)
END_MESSAGE_MAP()


BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	// ファイル選択ボタン(絵文字にする)
	GetDlgItem(IDC_BUTTON_BROWSEFILE)->SetWindowTextW(L"\U0001F4C4");


	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool SettingDialog::UpdateStatus()
{
	mHotKey = mHotKeyAttr.ToString();

	GetDlgItem(IDC_EDIT_FILERPATH)->EnableWindow(mIsUseExternalFiler);
	GetDlgItem(IDC_EDIT_FILERPARAM)->EnableWindow(mIsUseExternalFiler);
	GetDlgItem(IDC_BUTTON_BROWSEFILE)->EnableWindow(mIsUseExternalFiler);

	GetDlgItem(IDC_EDIT_ALPHA)->EnableWindow(mTransparencyType != 2);

	return true;
}

void SettingDialog::OnButtonHotKey()
{
	UpdateData();

	HotKeyDialog dlg(mHotKeyAttr);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(mHotKeyAttr);

	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnButtonBrowseFile()
{
	UpdateData();

	CString filterStr((LPCTSTR)IDS_FILTER_EXE);
	CFileDialog dlg(TRUE, NULL, mFilerPath, OFN_FILEMUSTEXIST, filterStr, this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mFilerPath = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);
}

void SettingDialog::OnCheckUseFilter()
{
	UpdateData();
	UpdateStatus();
}

void SettingDialog::OnCbnTransparencyChanged()
{
	UpdateData();
	UpdateStatus();
}

void SettingDialog::OnButtonShortcut()
{
	ShortcutDialog dlg;
	dlg.DoModal();
}

