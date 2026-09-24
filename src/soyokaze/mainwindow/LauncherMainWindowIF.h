#pragma once

class IconLabel;
class KeywordEdit;
class CandidateListCtrl;

namespace launcherapp { namespace mainwindow {

namespace guide {
class GuideCtrl;
}

	// ランチャーのメイン画面の部品を取得する機能を提供するI/F
class LauncherMainWindowIF
{
public:
	virtual ~LauncherMainWindowIF() {}

public:
	//
	virtual CWnd* GetWindowObject() = 0;

	/** メインウインドウの非表示要求をStateへ通知する */
	virtual void DeactivateWindow() = 0;

	// アイコンラベルウインドウを取得する
	virtual IconLabel* GetIconLabel() = 0;

	// 説明欄
	virtual CStatic* GetDescriptionLabel() = 0;

	// ガイド欄
	virtual guide::GuideCtrl* GetGuideLabel() = 0;

	// 入力欄
	virtual KeywordEdit* GetEdit() = 0;

	// 候補欄
	virtual CandidateListCtrl* GetCandidateList() = 0;
	
	// メインウインドウのフォント
	virtual CFont* GetMainWindowFont() = 0;
	// メインウインドウのフォント変更を通知する
	virtual void OnMainWindowFontChanged(CFont* font) = 0;

};


}
}

