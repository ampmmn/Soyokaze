#include "pch.h"
#include "framework.h"
#include "SoundSettingDialog.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 
class SoundSettingDialog : public CDialog
{
public:
	// 入力欄への文字入力時に再生するmp3ファイル
	CString mSoundFilePathInput;
	// 候補欄の選択変更入力時に再生するmp3ファイル
	CString mSoundFilePathSelect;
	// コマンド実行時に再生するmp3ファイル
	CString mSoundFilePathExecute;
	//
	Settings* mSettingsPtr{nullptr};

	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();

	bool UpdateStatus();

	bool SelectFile(CString& file);

	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonSoundFileInput();
	afx_msg void OnButtonSoundFileSelect();
	afx_msg void OnButtonSoundFileExecute();
};

bool SoundSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	return true;
}

bool SoundSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void SoundSettingDialog::OnOK()
{
	auto settingsPtr = mSettingsPtr;

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

BEGIN_MESSAGE_MAP(SoundSettingDialog, CDialog)
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

void SoundSettingDialog::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;

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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageSound::PImpl
{
	SoundSettingDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageSound)

AppSettingPageSound::AppSettingPageSound() : 
	AppSettingPageBase(_T("基本"), _T("効果音")),
	in(new PImpl)
{
}

AppSettingPageSound::~AppSettingPageSound()
{
}

// ウインドウを作成する
bool AppSettingPageSound::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_SOUNDSETTING, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageSound::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageSound::GetOrder()
{
	return 10;
}
// 
bool AppSettingPageSound::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageSound::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageSound::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageSound::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageSound::GetHelpPageId(String& id)
{
	id = "SoundSetting";
	return true;
}

