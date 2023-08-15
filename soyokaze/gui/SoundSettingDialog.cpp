#include "pch.h"
#include "framework.h"
#include "SoundSettingDialog.h"
#include "Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


SoundSettingDialog::SoundSettingDialog(CWnd* parentWnd) : 
	SettingPage(_T("効果音"), IDD_SOUNDSETTING, parentWnd)
{
}

SoundSettingDialog::~SoundSettingDialog()
{
}

BOOL SoundSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL SoundSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void SoundSettingDialog::OnOK()
{
	auto settingsPtr = (Settings*)GetParam();

	settingsPtr->Set(_T("Sound:FilePathInput"), mSoundFilePathInput);
	settingsPtr->Set(_T("Sound:FilePathSelect"), mSoundFilePathSelect);
	settingsPtr->Set(_T("Sound:FilePathExecute"), mSoundFilePathExecute);

	__super::OnOK();
}

void SoundSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_SOUNDINPUT, mSoundFilePathInput);
	DDX_Text(pDX, IDC_EDIT_SOUNDSELECT, mSoundFilePathSelect);
	DDX_Text(pDX, IDC_EDIT_SOUNDEXECUTE, mSoundFilePathExecute);
}

BEGIN_MESSAGE_MAP(SoundSettingDialog, SettingPage)
	ON_COMMAND(IDC_BUTTON_SOUNDINPUT, OnButtonSoundFileInput)
	ON_COMMAND(IDC_BUTTON_SOUNDSELECT, OnButtonSoundFileSelect)
	ON_COMMAND(IDC_BUTTON_SOUNDEXECUTE, OnButtonSoundFileExecute)
END_MESSAGE_MAP()


BOOL SoundSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool SoundSettingDialog::UpdateStatus()
{
	return true;
}

void SoundSettingDialog::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();

	mSoundFilePathInput = settingsPtr->Get(_T("Sound:FilePathInput"), _T(""));
	mSoundFilePathSelect = settingsPtr->Get(_T("Sound:FilePathSelect"), _T(""));
	mSoundFilePathExecute = settingsPtr->Get(_T("Sound:FilePathExecute"), _T(""));
}

bool SoundSettingDialog::SelectFile(CString& fileStr)
{
	UpdateData();

	CString filterStr((LPCTSTR)IDS_FILTER_SOUND);
	CFileDialog dlg(TRUE, NULL, fileStr, OFN_FILEMUSTEXIST, filterStr, this);
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	fileStr = dlg.GetPathName();
	UpdateStatus();
	UpdateData(FALSE);

	return true;
}

void SoundSettingDialog::OnButtonSoundFileInput()
{
	SelectFile(mSoundFilePathInput);
}

void SoundSettingDialog::OnButtonSoundFileSelect()
{
	SelectFile(mSoundFilePathSelect);
}

void SoundSettingDialog::OnButtonSoundFileExecute()
{
	SelectFile(mSoundFilePathExecute);
}

