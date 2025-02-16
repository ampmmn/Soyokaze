#pragma once

class IconLabel;
class KeywordEdit;
class CandidateListCtrl;

namespace launcherapp {
namespace mainwindow {

	// ランチャーのメイン画面の部品を取得する機能を提供するI/F
class LauncherMainWindowIF
{
public:
	virtual ~LauncherMainWindowIF() {}

public:
	//
	virtual CWnd* GetWindowObject() = 0;

	// アイコンラベルウインドウを取得する
	virtual IconLabel* GetIconLabel() = 0;

	// 説明欄
	virtual CStatic* GetDescriptionLabel() = 0;

	// ガイド欄
	virtual CStatic* GetGuideLabel() = 0;

	// 入力欄
	virtual KeywordEdit* GetEdit() = 0;

	// 候補欄
	virtual CandidateListCtrl* GetCandidateList() = 0;
	
	// メインウインドウのフォント
	virtual CFont* GetMainWindowFont() = 0;

};


}
}

