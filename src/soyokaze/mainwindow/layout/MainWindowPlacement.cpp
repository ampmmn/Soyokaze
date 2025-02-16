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

// $B%"%$%3%sMs(B
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

// $B@bL@Ms(B
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


// $B%,%$%IMs(B
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

// $BF~NOMs(B
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

// $B8uJdMs(B
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

// $B>e$NM>Gr(B
int MainWindowPlacement::GetMarginTop() { return 2; }
// $B:8$NM>Gr(B
int MainWindowPlacement::GetMarginLeft() { return 2; }
// $BF~NOMs$H8uJdMs$NM>Gr(B
int MainWindowPlacement::GetMarginEditToList() { return 3; }

// $BF~NO2hLL$N%U%)%s%H%5%$%:(B
int MainWindowPlacement::GetFontPixelSize()
{
	auto font = mMainWnd->GetMainWindowFont();

	LOGFONT lf;
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

