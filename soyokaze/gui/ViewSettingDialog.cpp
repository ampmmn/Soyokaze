#include "pch.h"
#include "framework.h"
#include "ViewSettingDialog.h"
#include "Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


struct ViewSettingDialog::PImpl
{
	// 入力画面を常に最前面に表示
	BOOL mIsTopMost;

	// アクティブ状態でなくなったらウインドウを隠す
	BOOL mIsHideOnInactive;

	// 半透明の表示方法
	int mTransparencyType;
	// 半透明表示の透明度
	UINT mAlpha;

	// マウスカーソル位置に入力欄を表示する
	BOOL mIsShowMainWindowOnCursor;

	// コマンド種別を表示するか?
	BOOL mIsShowCommandType;

	// 候補欄の背景色を交互に変える
	BOOL mIsAlternateColor;

	// 候補欄の各項目にアイコンを描画するか
	BOOL mIsDrawIconOnCandidate;

	// 入力画面の初期状態時にコメント表示欄に表示する文字列
	CString mDefaultComment;
};

ViewSettingDialog::ViewSettingDialog(CWnd* parentWnd) : 
	SettingPage(_T("表示"), IDD_VIEWSETTING, parentWnd),
	in(std::make_unique<PImpl>())
{
	in->mAlpha = 128;
}

ViewSettingDialog::~ViewSettingDialog()
{
}

BOOL ViewSettingDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL ViewSettingDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void ViewSettingDialog::OnOK()
{
	auto settingsPtr = (Settings*)GetParam();
	settingsPtr->Set(_T("Soyokaze:TopMost"), (bool)in->mIsTopMost);
	settingsPtr->Set(_T("Soyokaze:IsHideOnInactive"), (bool)in->mIsHideOnInactive);

	if (in->mTransparencyType == 0) {
		settingsPtr->Set(_T("WindowTransparency:Enable"), true);
		settingsPtr->Set(_T("WindowTransparency:InactiveOnly"), true);
	}
	else if (in->mTransparencyType == 1) {
		settingsPtr->Set(_T("WindowTransparency:Enable"), true);
		settingsPtr->Set(_T("WindowTransparency:InactiveOnly"), false);
	}
	else {
		settingsPtr->Set(_T("WindowTransparency:Enable"), false);
		settingsPtr->Set(_T("WindowTransparency:InactiveOnly"), true);
	}

	settingsPtr->Set(_T("WindowTransparency:Alpha"), (int)in->mAlpha);

	settingsPtr->Set(_T("Soyokaze:DefaultComment"), in->mDefaultComment);

	settingsPtr->Set(_T("Soyokaze:IsShowMainWindowOnCurorPos"), (bool)in->mIsShowMainWindowOnCursor);
	settingsPtr->Set(_T("Soyokaze:IsShowCommandType"), (bool)in->mIsShowCommandType);
	settingsPtr->Set(_T("Soyokaze:IsAlternateColor"), (bool)in->mIsAlternateColor);
	settingsPtr->Set(_T("Soyokaze:IsDrawIconOnCandidate"), (bool)in->mIsDrawIconOnCandidate);

	__super::OnOK();
}

void ViewSettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_CHECK_TOPMOST, in->mIsTopMost);
	DDX_Check(pDX, IDC_CHECK_HIDEONINACTIVE, in->mIsHideOnInactive);
	DDX_CBIndex(pDX, IDC_COMBO_TRANSPARENCY, in->mTransparencyType);
	DDX_Text(pDX, IDC_EDIT_ALPHA, in->mAlpha);
	DDV_MinMaxInt(pDX, in->mAlpha, 0, 255);
	DDX_Text(pDX, IDC_EDIT_DEFAULTCOMMENT, in->mDefaultComment);
	DDX_Check(pDX, IDC_CHECK_MOVETOCURSOR, in->mIsShowMainWindowOnCursor);
	DDX_Check(pDX, IDC_CHECK_SHOWCOMMANDTYPE, in->mIsShowCommandType);
	DDX_Check(pDX, IDC_CHECK_ALTERNATELISTCOLOR, in->mIsAlternateColor);
	DDX_Check(pDX, IDC_CHECK_DRAWICONONCANDIDATE, in->mIsDrawIconOnCandidate);
}

BEGIN_MESSAGE_MAP(ViewSettingDialog, SettingPage)
	ON_CBN_SELCHANGE(IDC_COMBO_TRANSPARENCY, OnCbnTransparencyChanged)
END_MESSAGE_MAP()


BOOL ViewSettingDialog::OnInitDialog()
{
	__super::OnInitDialog();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

bool ViewSettingDialog::UpdateStatus()
{
	GetDlgItem(IDC_EDIT_ALPHA)->EnableWindow(in->mTransparencyType != 2);

	return true;
}


void ViewSettingDialog::OnCbnTransparencyChanged()
{
	UpdateData();
	UpdateStatus();
}

void ViewSettingDialog::OnEnterSettings()
{
	auto settingsPtr = (Settings*)GetParam();
	in->mIsTopMost = settingsPtr->Get(_T("Soyokaze:TopMost"), false);
	in->mIsHideOnInactive = settingsPtr->Get(_T("Soyokaze:IsHideOnInactive"), false);

	if (settingsPtr->Get(_T("WindowTransparency:Enable"), false) == false) {
		in->mTransparencyType = 2;
	}
	else if (settingsPtr->Get(_T("WindowTransparency:InactiveOnly"), true)) {
		in->mTransparencyType = 0;
	}
	else {
		in->mTransparencyType = 1;
	}

	in->mAlpha = settingsPtr->Get(_T("WindowTransparency:Alpha"), 128);
	if (in->mAlpha < 0) { in->mAlpha = 0; }
	if (in->mAlpha > 255) { in->mAlpha = 255; }

	CString defStr((LPCTSTR)ID_STRING_DEFAULTDESCRIPTION);
	in->mDefaultComment = settingsPtr->Get(_T("Soyokaze:DefaultComment"), defStr);
	in->mIsShowMainWindowOnCursor = settingsPtr->Get(_T("Soyokaze:IsShowMainWindowOnCurorPos"), false);
	in->mIsShowCommandType = settingsPtr->Get(_T("Soyokaze:IsShowCommandType"), true);
	in->mIsAlternateColor = settingsPtr->Get(_T("Soyokaze:IsAlternateColor"), false);
	in->mIsDrawIconOnCandidate = settingsPtr->Get(_T("Soyokaze:IsDrawIconOnCandidate"), false);
}
