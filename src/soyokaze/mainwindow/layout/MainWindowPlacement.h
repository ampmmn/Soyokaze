#pragma once

#include "resource.h"

namespace launcherapp {
namespace mainwindow {
namespace layout {

class MainWindowPlacement
{
public:
	MainWindowPlacement() : 
		mParent(nullptr),
		mIcon(nullptr),
		mDesc(nullptr),
		mGuide(nullptr),
		mEdit(nullptr),
		mCandidateList(nullptr)
	{
	}

	void SetMainWindowHwnd(HWND hwnd) 
	{
		mParent = CWnd::FromHandle(hwnd);
	}

// 入力欄
	int GetMainWindowWidth() {
		CRect rc;
		mParent->GetClientRect(&rc);
		return rc.Width();
	}
	int GetMainWindowHeight() {
		CRect rc;
		mParent->GetClientRect(&rc);
		return rc.Height();
	}

// アイコン欄
	CWnd* GetIconLabel() {
		if (mIcon == nullptr) {
			mIcon = mParent->GetDlgItem(IDC_STATIC_ICON);
		}
		return mIcon;
	}
	int GetIconWindowWidth() {
		auto iconLabel = GetIconLabel();
		if (iconLabel == nullptr) {
			return -1;
		}
		CRect rc;
		iconLabel->GetWindowRect(&rc);
		return rc.Width();
	}
	int GetIconWindowHeight() {
		auto iconLabel = GetIconLabel();
		if (iconLabel == nullptr) {
			return -1;
		}
		CRect rc;
		iconLabel->GetWindowRect(&rc);
		return rc.Height();
	}

// 説明欄
	CWnd* GetDescriptionLabel() {
		if (mDesc == nullptr) {
			mDesc = mParent->GetDlgItem(IDC_STATIC_DESCRIPTION);
		}
		return mDesc;
	}
	int GetDescriptionWindowWidth()
	{
		auto label = GetDescriptionLabel();
		if (label == nullptr) {
			return -1;
		}
		CRect rc;
		label->GetWindowRect(&rc);
		return rc.Width();
	}

	int GetDescriptionWindowHeight()
	{
		auto label = GetDescriptionLabel();
		if (label == nullptr) {
			return -1;
		}
		CRect rc;
		label->GetWindowRect(&rc);
		return rc.Height();
	}


// ガイド欄
	CWnd* GetGuideLabel()
	{
		if (mGuide == nullptr) {
			mGuide = mParent->GetDlgItem(IDC_STATIC_GUIDE);
		}
		return mGuide;
	}
	int GetGuideWindowWidth()
	{
		auto label = GetGuideLabel();
		if (label == nullptr) {
			return -1;
		}
		CRect rc;
		label->GetWindowRect(&rc);
		return rc.Width();
	}
	int GetGuideWindowHeight()
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
	CWnd* GetEdit()
	{
		if (mEdit == nullptr) {
			mEdit = mParent->GetDlgItem(IDC_EDIT_COMMAND);
		}
		return mEdit;
	}

	int GetEditWindowWidth() {
		auto edit = GetEdit();
		if (edit == nullptr) {
			return -1;
		}
		CRect rc;
		edit->GetWindowRect(&rc);
		return rc.Width();
	}
	int GetEditWindowHeight() {
		auto edit = GetEdit();
		if (edit == nullptr) {
			return -1;
		}
		CRect rc;
		edit->GetWindowRect(&rc);
		return rc.Height();
	}

// 候補欄
	CWnd* GetCandidateList() {
		if (mCandidateList== nullptr) {
			mCandidateList = mParent->GetDlgItem(IDC_LIST_CANDIDATE);
		}
		return mCandidateList;
	}
	int GetCandidateListWindowWidth() {
		auto list = GetCandidateList();
		if (list == nullptr) {
			return -1;
		}
		CRect rc;
		list->GetWindowRect(&rc);
		return rc.Width();
	}
	int GetCandidateListWindowHeight() {
		auto list = GetCandidateList();
		if (list == nullptr) {
			return -1;
		}
		CRect rc;
		list->GetWindowRect(&rc);
		return rc.Height();
	}

	// 上の余白
	int GetMarginTop() { return 2; }
	// 左の余白
	int GetMarginLeft() { return 2; }
	// 入力欄と候補欄の余白
	int GetMarginEditToList() { return 3; }


// 
private:
	CWnd* mParent = nullptr;
	CWnd* mIcon = nullptr;
	CWnd* mDesc = nullptr;
	CWnd* mGuide = nullptr;
	CWnd* mEdit = nullptr;
	CWnd* mCandidateList = nullptr;


};


}
}
}
