#include "pch.h"
#include "framework.h"
#include "ViewSettingDialog.h"
#include "setting/Settings.h"
#include "app/Manual.h"
#include "icon/IconLoader.h"
#include "icon/IconLabelForApp.h"
#include "icon/AppIcon.h"
#include "utility/Path.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::icon;

constexpr LPCTSTR DEFAULTFONTNAME = _T("Tahoma");

class ViewSettingDialog : public CDialog
{
public:
	~ViewSettingDialog();

	void OnEnterSettings(Settings* settingsPtr);
	bool OnSetActive();
	bool OnKillActive();

	void SetIconPath(const CString& appIconPath);
	bool UpdateStatus();

	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg void OnNotifyLinkOpen(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnButtonBrowse();
	afx_msg void OnButtonResetIcon();
	afx_msg void OnButtonResetFont();
	afx_msg void OnCbnKillfocusFontSize();

	// 入力欄のアイコンを表示する
	BOOL mIsDrawIcon{TRUE};
	// 入力欄のプレースホルダーを表示する
	BOOL mIsDrawPlaceHolder{TRUE};

	// 半透明の表示方法
	int mTransparencyType{0};
	// 半透明表示の透明度
	UINT mAlpha{128};

	// コマンド種別を表示するか?
	BOOL mIsShowCommandType{TRUE};

	// 操作ガイドを表示するか?
	BOOL mIsShowGuide{TRUE};

	// 候補欄の背景色を交互に変える
	BOOL mIsAlternateColor{TRUE};

	// 候補欄の各項目にアイコンを描画するか
	BOOL mIsDrawIconOnCandidate{TRUE};

	// 入力画面の初期状態時にコメント表示欄に表示する文字列
	CString mDefaultComment;

	// フォントサイズ
	int mFontSize{9};

	// アプリアイコン
	HICON mIcon{nullptr};
	CString mAppIconFilePath;
	IconLabelForApp mIconLabelPtr;
	bool mIsIconReset{false};

	Settings* mSettingsPtr{nullptr};
};

ViewSettingDialog::~ViewSettingDialog()
{
	if (mIcon) {
		DestroyIcon(mIcon);
		mIcon = nullptr;
	}
}

bool ViewSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	return true;
}

bool ViewSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void ViewSettingDialog::OnOK()
{
	if (UpdateData() == FALSE) {
		return;
	}

	auto settingsPtr = mSettingsPtr;
	settingsPtr->Set(_T("ViewSetting:IsDrawIcon"), (bool)mIsDrawIcon);
	settingsPtr->Set(_T("ViewSetting:IsDrawPlaceHolder"), (bool)mIsDrawPlaceHolder);

	if (mTransparencyType == 0) {
		settingsPtr->Set(_T("WindowTransparency:Enable"), true);
		settingsPtr->Set(_T("WindowTransparency:InactiveOnly"), true);
	}
	else if (mTransparencyType == 1) {
		settingsPtr->Set(_T("WindowTransparency:Enable"), true);
		settingsPtr->Set(_T("WindowTransparency:InactiveOnly"), false);
	}
	else {
		settingsPtr->Set(_T("WindowTransparency:Enable"), false);
		settingsPtr->Set(_T("WindowTransparency:InactiveOnly"), true);
	}

	settingsPtr->Set(_T("WindowTransparency:Alpha"), (int)mAlpha);

	settingsPtr->Set(_T("Soyokaze:DefaultComment"), mDefaultComment);

	settingsPtr->Set(_T("Soyokaze:IsShowCommandType"), (bool)mIsShowCommandType);
	settingsPtr->Set(_T("Soyokaze:IsShowGuide"), (bool)mIsShowGuide);
	settingsPtr->Set(_T("Soyokaze:IsAlternateColor"), (bool)mIsAlternateColor);
	settingsPtr->Set(_T("Soyokaze:IsDrawIconOnCandidate"), (bool)mIsDrawIconOnCandidate);

	CMFCFontComboBox* fontCombo = (CMFCFontComboBox*)GetDlgItem(IDC_MFCFONTCOMBO_MAIN);
	ASSERT(fontCombo);
	auto fontInfo = fontCombo->GetSelFont();

	settingsPtr->Set(_T("MainWindow:FontName"), fontInfo->m_strName);

	if (mFontSize < 6) { mFontSize = 6; }
	else if (mFontSize > 128) { mFontSize = 128; }

	settingsPtr->Set(_T("MainWindow:FontSize"), mFontSize);

	// アプリアイコンが設定(変更)された場合は上書き
	Path icon(mAppIconFilePath);
	if (mIsIconReset == false && icon.FileExists()) {
		AppIcon::Get()->Import(mAppIconFilePath);
	}
	else if (mIsIconReset) {
		AppIcon::Get()->Reset();
	}


	__super::OnOK();
}

void ViewSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_DRAWICON, mIsDrawIcon);
	DDX_Check(pDX, IDC_CHECK_DRAWPLACEHOLDER, mIsDrawPlaceHolder);
	DDX_CBIndex(pDX, IDC_COMBO_TRANSPARENCY, mTransparencyType);
	DDX_Text(pDX, IDC_EDIT_ALPHA, mAlpha);
	DDV_MinMaxInt(pDX, mAlpha, 0, 255);
	DDX_Text(pDX, IDC_EDIT_DEFAULTCOMMENT, mDefaultComment);
	DDX_Check(pDX, IDC_CHECK_SHOWCOMMANDTYPE, mIsShowCommandType);
	DDX_Check(pDX, IDC_CHECK_SHOWGUIDE, mIsShowGuide);
	DDX_Check(pDX, IDC_CHECK_ALTERNATELISTCOLOR, mIsAlternateColor);
	DDX_Check(pDX, IDC_CHECK_DRAWICONONCANDIDATE, mIsDrawIconOnCandidate);
	DDX_Text(pDX, IDC_COMBO_FONTSIZE, mFontSize);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(ViewSettingDialog, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_TRANSPARENCY, OnUpdateStatus)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_MACRO, OnNotifyLinkOpen)
	ON_COMMAND(IDC_BUTTON_BROWSE, OnButtonBrowse)
	ON_COMMAND(IDC_BUTTON_RESETICON, OnButtonResetIcon)
	ON_COMMAND(IDC_BUTTON_RESETFONT, OnButtonResetFont)
	ON_COMMAND(IDC_CHECK_DRAWICON, OnUpdateStatus)
	ON_CBN_KILLFOCUS(IDC_COMBO_FONTSIZE, OnCbnKillfocusFontSize)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL ViewSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	mIconLabelPtr.SubclassDlgItem(IDC_STATIC_ICON, this);
	mIconLabelPtr.DisableIconChange();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

void ViewSettingDialog::SetIconPath(const CString& appIconPath)
{
	bool isShared = false;
	HICON h = IconLoader::Get()->LoadIconFromImageFile(appIconPath, isShared);
	if (h == nullptr) {
		AfxMessageBox(_T("ファイルのロードに失敗しました"));
		return ;
	}

	if (mIcon) {
		DestroyIcon(mIcon);
	}
	mIcon = h;
	mAppIconFilePath = appIconPath;
	mIsIconReset = false;

	// 再描画
	mIconLabelPtr.DrawIcon(mIcon);
}

bool ViewSettingDialog::UpdateStatus()
{
	GetDlgItem(IDC_EDIT_ALPHA)->EnableWindow(mTransparencyType != 2);

	// フォントサイズの有効範囲を超えていたら範囲内に丸める
	if (mFontSize < 6) { mFontSize = 6; }
	else if (mFontSize > 128) { mFontSize = 128; }

	bool isDrawIcon = mIsDrawIcon != FALSE;
	GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(isDrawIcon);
	GetDlgItem(IDC_BUTTON_RESETICON)->EnableWindow(isDrawIcon);

	if (mIcon) {
		mIconLabelPtr.DrawIcon(mIcon);
	}
	else {
		auto appIcon = AppIcon::Get();
		auto iconHandle = (mIsIconReset) ? appIcon->DefaultIconHandle() : appIcon->IconHandle();
		mIconLabelPtr.DrawIcon(iconHandle);
	}

	return true;
}


void ViewSettingDialog::OnUpdateStatus()
{
	UpdateData();
	UpdateStatus();
}

void ViewSettingDialog::OnEnterSettings(Settings* settingsPtr)
{
	mSettingsPtr = settingsPtr;

	mIsDrawIcon = settingsPtr->Get(_T("ViewSetting:IsDrawIcon"), true);
	mIsDrawPlaceHolder = settingsPtr->Get(_T("ViewSetting:IsDrawPlaceHolder"), true);

	if (settingsPtr->Get(_T("WindowTransparency:Enable"), false) == false) {
		mTransparencyType = 2;
	}
	else if (settingsPtr->Get(_T("WindowTransparency:InactiveOnly"), true)) {
		mTransparencyType = 0;
	}
	else {
		mTransparencyType = 1;
	}

	mAlpha = settingsPtr->Get(_T("WindowTransparency:Alpha"), 128);
	if (mAlpha < 0) { mAlpha = 0; }
	if (mAlpha > 255) { mAlpha = 255; }

	CString defStr((LPCTSTR)ID_STRING_DEFAULTDESCRIPTION);
	mDefaultComment = settingsPtr->Get(_T("Soyokaze:DefaultComment"), defStr);
	mIsShowCommandType = settingsPtr->Get(_T("Soyokaze:IsShowCommandType"), true);
	mIsShowGuide = settingsPtr->Get(_T("Soyokaze:IsShowGuide"), true);
	mIsAlternateColor = settingsPtr->Get(_T("Soyokaze:IsAlternateColor"), true);
	mIsDrawIconOnCandidate = settingsPtr->Get(_T("Soyokaze:IsDrawIconOnCandidate"), true);

	CString fontName = settingsPtr->Get(_T("MainWindow:FontName"), DEFAULTFONTNAME);
	CMFCFontComboBox* fontCombo = (CMFCFontComboBox*)GetDlgItem(IDC_MFCFONTCOMBO_MAIN);
	ASSERT(fontCombo);
	fontCombo->SelectFont(fontName);
	mFontSize = settingsPtr->Get(_T("MainWindow:FontSize"), 9);
}

// マニュアル表示
void ViewSettingDialog::OnNotifyLinkOpen(
	NMHDR *pNMHDR,
 	LRESULT *pResult
)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	auto manual = launcherapp::app::Manual::GetInstance();
	manual->Navigate(_T("MacroList"));
	*pResult = 0;
}

// アイコン変更ボタン押下時の処理
void ViewSettingDialog::OnButtonBrowse()
{
	UpdateData();

	CString filterStr((LPCTSTR)IDS_FILTER_ICONIMAGEFILES);

	Path iconPath(Path::MODULEFILEPATH);
	iconPath.Shrink();

	if (mAppIconFilePath.IsEmpty() == FALSE) {
		iconPath = mAppIconFilePath;
	}

	CFileDialog dlg(TRUE, NULL, iconPath, OFN_FILEMUSTEXIST, filterStr, this);
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	SetIconPath(dlg.GetPathName());
}


void ViewSettingDialog::OnButtonResetIcon()
{
	UpdateData();

	// アイコンを初期状態に戻す
	if (mIcon) {
		DestroyIcon(mIcon);
		mIcon = nullptr;
	}
	mAppIconFilePath.Empty();
	mIsIconReset = true;

	UpdateStatus();

}

void ViewSettingDialog::OnButtonResetFont()
{
	UpdateData();

	mFontSize = 9;
	CMFCFontComboBox* fontCombo = (CMFCFontComboBox*)GetDlgItem(IDC_MFCFONTCOMBO_MAIN);
	ASSERT(fontCombo);
	fontCombo->SelectFont(DEFAULTFONTNAME);


	UpdateData(FALSE);
}

void ViewSettingDialog::OnCbnKillfocusFontSize()
{
	UpdateData();
	UpdateStatus();
	UpdateData(FALSE);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageView::PImpl
{
	ViewSettingDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageView)

AppSettingPageView::AppSettingPageView() : 
	AppSettingPageBase(_T(""), _T("表示")),
	in(new PImpl)
{
}

AppSettingPageView::~AppSettingPageView()
{
}

// ウインドウを作成する
bool AppSettingPageView::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_VIEWSETTING, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageView::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageView::GetOrder()
{
	return 70;
}
// 
bool AppSettingPageView::OnEnterSettings()
{
	in->mWindow.OnEnterSettings((Settings*)GetParam());
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageView::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageView::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageView::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageView::GetHelpPageId(CString& id)
{
	id = _T("InputWindowSetting");
	return true;
}

