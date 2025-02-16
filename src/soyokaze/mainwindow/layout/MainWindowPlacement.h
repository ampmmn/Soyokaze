#pragma once

#include "resource.h"
#include "mainwindow/LauncherMainWindowIF.h"

namespace launcherapp {
namespace mainwindow {
namespace layout {

class MainWindowPlacement
{
	using LauncherMainWindowIF = launcherapp::mainwindow::LauncherMainWindowIF; 
public:
	MainWindowPlacement(LauncherMainWindowIF* mainWnd);
	~MainWindowPlacement();

// 入力欄
	int GetMainWindowWidth();
	int GetMainWindowHeight();
// アイコン欄
	CWnd* GetIconLabel();
	int GetIconWindowWidth();
	int GetIconWindowHeight();

// 説明欄
	CWnd* GetDescriptionLabel();
	int GetDescriptionWindowWidth();
	int GetDescriptionWindowHeight();

// ガイド欄
	CWnd* GetGuideLabel();
	int GetGuideWindowWidth();
	int GetGuideWindowHeight();

// 入力欄
	CWnd* GetEdit();
	int GetEditWindowWidth();
	int GetEditWindowHeight();

// 候補欄
	CWnd* GetCandidateList();
	int GetCandidateListWindowWidth();
	int GetCandidateListWindowHeight();
	// 上の余白
	int GetMarginTop();
	// 左の余白
	int GetMarginLeft();
	// 入力欄と候補欄の余白
	int GetMarginEditToList();

	// 入力画面のフォントサイズ
	int GetFontPixelSize();

private:
	CWnd* GetParent();
// 
private:
	LauncherMainWindowIF* mMainWnd = nullptr;
	CWnd* mIcon = nullptr;
	CWnd* mDesc = nullptr;
	CWnd* mGuide = nullptr;
	CWnd* mEdit = nullptr;
	CWnd* mCandidateList = nullptr;
};


}
}
}
