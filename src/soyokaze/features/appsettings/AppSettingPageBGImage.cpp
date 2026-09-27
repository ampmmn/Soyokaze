#include "pch.h"
#include "framework.h"
#include "AppSettingPageBGImage.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class BGImageSettingDialog : public CDialog
{
public:
	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();
	bool UpdateStatus();

	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg void OnButtonBrowseFile();

	BOOL mIsEnable{FALSE};
	CString mFilePath;
	int mAlpha{0};
	int mPosition{0};
	Settings* mSettingsPtr{nullptr};
};

void BGImageSettingDialog::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;
	mIsEnable = settingsPtr->Get(_T("BGImage:Enable"), false);
	mFilePath = settingsPtr->Get(_T("BGImage:BGImageFilePath"), _T(""));
	mAlpha = settingsPtr->Get(_T("BGImage:Alpha"), 0);
	mPosition = settingsPtr->Get(_T("BGImage:Position"), 0);

	if (mAlpha < 0) {
		mAlpha = 0;
	}
	else if (mAlpha > 100) {
		mAlpha = 100;
	}

	if (mPosition < 0 || mPosition > 6) {
		mPosition = 0;
	}
}

bool BGImageSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

bool BGImageSettingDialog::OnKillActive()
{
	return UpdateData() != FALSE;
}

void BGImageSettingDialog::OnOK()
{
	if (UpdateData() == FALSE) {
		return;
	}

	mSettingsPtr->Set(_T("BGImage:Enable"), (bool)mIsEnable);
	mSettingsPtr->Set(_T("BGImage:BGImageFilePath"), mFilePath);
	mSettingsPtr->Set(_T("BGImage:Alpha"), mAlpha);
	mSettingsPtr->Set(_T("BGImage:Position"), mPosition);

	__super::OnOK();
}

void BGImageSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_USEBGIMAGE, mIsEnable);
	DDX_Text(pDX, IDC_EDIT_FILEPATH, mFilePath);
	DDX_CBIndex(pDX, IDC_COMBO_POSITION, mPosition);
	DDX_Text(pDX, IDC_EDIT_ALPHA, mAlpha);
	DDV_MinMaxInt(pDX, mAlpha, 0, 100);
}

BEGIN_MESSAGE_MAP(BGImageSettingDialog, CDialog)
	ON_COMMAND(IDC_CHECK_USEBGIMAGE, OnUpdateStatus)
	ON_COMMAND(IDC_BUTTON_BROWSEFILE, OnButtonBrowseFile)
END_MESSAGE_MAP()

BOOL BGImageSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

bool BGImageSettingDialog::UpdateStatus()
{
	bool isEnable = mIsEnable != FALSE;
	GetDlgItem(IDC_EDIT_FILEPATH)->EnableWindow(isEnable);
	GetDlgItem(IDC_BUTTON_BROWSEFILE)->EnableWindow(isEnable);
	GetDlgItem(IDC_COMBO_POSITION)->EnableWindow(isEnable);
	GetDlgItem(IDC_EDIT_ALPHA)->EnableWindow(isEnable);
	return true;
}

void BGImageSettingDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

void BGImageSettingDialog::OnButtonBrowseFile()
{
	UpdateData();

	CFileDialog dlg(
		TRUE,
		nullptr,
		mFilePath,
		OFN_FILEMUSTEXIST,
		_T("画像ファイル (*.bmp;*.jpg;*.jpeg;*.png;*.gif)|*.bmp;*.jpg;*.jpeg;*.png;*.gif|BMPファイル (*.bmp)|*.bmp|JPEGファイル (*.jpg;*.jpeg)|*.jpg;*.jpeg|PNGファイル (*.png)|*.png|GIFファイル (*.gif)|*.gif||"),
		this
	);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	mFilePath = dlg.GetPathName();
	UpdateData(FALSE);
}

struct AppSettingPageBGImage::PImpl
{
	BGImageSettingDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageBGImage)

AppSettingPageBGImage::AppSettingPageBGImage() :
	AppSettingPageBase(_T("表示"), _T("背景画像")),
	in(new PImpl)
{
}

AppSettingPageBGImage::~AppSettingPageBGImage()
{
}

bool AppSettingPageBGImage::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_BGIMAGE, CWnd::FromHandle(parentWindow)) != FALSE;
}

HWND AppSettingPageBGImage::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

int AppSettingPageBGImage::GetOrder()
{
	return 60;
}

bool AppSettingPageBGImage::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

bool AppSettingPageBGImage::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

bool AppSettingPageBGImage::OnKillActive()
{
	return in->mWindow.OnKillActive();
}

void AppSettingPageBGImage::OnOKCall()
{
	in->mWindow.OnOK();
}

bool AppSettingPageBGImage::GetHelpPageId(String& helpPageId)
{
	helpPageId = "BGImageSetting";
	return true;
}
