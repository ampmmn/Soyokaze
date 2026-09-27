#include "pch.h"
#include "MainWindowPlacement.h"

namespace launcherapp {
namespace mainwindow {
namespace layout {

MainWindowPlacement::MainWindowPlacement(LauncherMainWindowIF* mainWnd) : 
	mMainWnd(mainWnd), mIcon(nullptr), mDesc(nullptr), mGuide(nullptr), mEdit(nullptr), mCandidateList(nullptr)
{
}

MainWindowPlacement::~MainWindowPlacement()
{
}

int MainWindowPlacement::GetMainWindowWidth()
{
	CRect rc;
	GetParent()->GetClientRect(&rc);
	return rc.Width();
}

int MainWindowPlacement::GetMainWindowHeight()
{
	CRect rc;
	GetParent()->GetClientRect(&rc);
	return rc.Height();
}

// アイコン欄
CWnd* MainWindowPlacement::GetIconLabel()
{
	if (mIcon == nullptr) {
		mIcon = GetParent()->GetDlgItem(IDC_STATIC_ICON);
	}
	return mIcon;
}

int MainWindowPlacement::GetIconWindowWidth()
{
	auto iconLabel = GetIconLabel();
	if (iconLabel == nullptr) {
		return -1;
	}
	CRect rc;
	iconLabel->GetWindowRect(&rc);
	return rc.Width();
}

int MainWindowPlacement::GetIconWindowHeight()
{
	auto iconLabel = GetIconLabel();
	if (iconLabel == nullptr) {
		return -1;
	}
	CRect rc;
	iconLabel->GetWindowRect(&rc);
	return rc.Height();
}

// 説明欄
CWnd* MainWindowPlacement::GetDescriptionLabel()
{
	if (mDesc == nullptr) {
		mDesc = GetParent()->GetDlgItem(IDC_STATIC_DESCRIPTION);
	}
	return mDesc;
}

int MainWindowPlacement::GetDescriptionWindowWidth()
{
	auto label = GetDescriptionLabel();
	if (label == nullptr) {
		return -1;
	}
	CRect rc;
	label->GetWindowRect(&rc);
	return rc.Width();
}

int MainWindowPlacement::GetDescriptionWindowHeight()
{
	auto label = GetDescriptionLabel();
	if (label == nullptr) {
		return -1;
	}
	CRect rc;
	label->GetWindowRect(&rc);
	return rc.Height();
}

CWnd* MainWindowPlacement::GetOptionButton()
{
	return GetParent()->GetDlgItem(IDC_BUTTON_OPTION);
}

// ガイド欄
CWnd* MainWindowPlacement::GetGuideLabel()
{
	if (mGuide == nullptr) {
		mGuide = GetParent()->GetDlgItem(IDC_STATIC_GUIDE);
	}
	return mGuide;
}

int MainWindowPlacement::GetGuideWindowWidth()
{
	auto label = GetGuideLabel();
	if (label == nullptr) {
		return -1;
	}
	CRect rc;
	label->GetWindowRect(&rc);
	return rc.Width();
}

int MainWindowPlacement::GetGuideWindowHeight()
{
	auto label = GetGuideLabel();
	if (label == nullptr) {
		return -1;
	}
	CRect rc;
	label->GetWindowRect(&rc);
	return rc.Height();
}

// 入力欄
CWnd* MainWindowPlacement::GetEdit()
{
	if (mEdit == nullptr) {
		mEdit = GetParent()->GetDlgItem(IDC_EDIT_COMMAND);
	}
	return mEdit;
}

int MainWindowPlacement::GetEditWindowWidth()
{
	auto edit = GetEdit();
	if (edit == nullptr) {
		return -1;
	}
	CRect rc;
	edit->GetWindowRect(&rc);
	return rc.Width();
}

int MainWindowPlacement::GetEditWindowHeight()
{
	auto edit = GetEdit();
	if (edit == nullptr) {
		return -1;
	}
	CRect rc;
	edit->GetWindowRect(&rc);
	return rc.Height();
}

// 候補欄
CWnd* MainWindowPlacement::GetCandidateList()
{
	if (mCandidateList== nullptr) {
		mCandidateList = GetParent()->GetDlgItem(IDC_LIST_CANDIDATE);
	}
	return mCandidateList;
}

int MainWindowPlacement::GetCandidateListWindowWidth()
{
	auto list = GetCandidateList();
	if (list == nullptr) {
		return -1;
	}
	CRect rc;
	list->GetWindowRect(&rc);
	return rc.Width();
}

int MainWindowPlacement::GetCandidateListWindowHeight()
{
	auto list = GetCandidateList();
	if (list == nullptr) {
		return -1;
	}
	CRect rc;
	list->GetWindowRect(&rc);
	return rc.Height();
}

// 上の余白
int MainWindowPlacement::GetMarginTop() { return 2; }
// 左の余白
int MainWindowPlacement::GetMarginLeft() { return 2; }
// 入力欄と候補欄の余白
int MainWindowPlacement::GetMarginEditToList() { return 3; }

// 入力画面のフォントサイズ
int MainWindowPlacement::GetFontPixelSize()
{
	auto font = mMainWnd->GetMainWindowFont();

	LOGFONT lf = {};
	if (font->GetSafeHandle() == nullptr) {
		// フォントハンドルがない場合は気休めにデフォルト値(16)を返す
		return 16;
	}
	font->GetLogFont(&lf);	

	CClientDC dc(GetParent());
	auto orgFont = dc.SelectObject(font);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int fontH = tm.tmHeight + tm.tmInternalLeading + tm.tmExternalLeading;
	dc.SelectObject(orgFont);

	return fontH;
}

CWnd* MainWindowPlacement::GetParent()
{
	return mMainWnd->GetWindowObject();
}

}
}
}

