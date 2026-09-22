#pragma once

#include <memory>
#include <vector>
#include "commands/core/CommandIF.h"

class ExtraCandidatePopupWnd;

/**
  パラメータ入力用の追加候補を表示するポップアップリスト
*/
class ExtraCandidateListCtrl : public CListCtrl
{
public:
	ExtraCandidateListCtrl();
	~ExtraCandidateListCtrl() override;

	bool CreatePopup(CWnd* owner);
	// Popupとリストにフォントを反映する
	void SetPopupFont(CFont* font);
	// 現在のフォントに基づく行高を取得する
	int GetRowHeight() const;
	void SetCandidates(const std::vector<RefPtr<launcherapp::core::Command>>& candidates);
	void SelectByName(const CString& name);
	void ShowAt(const CPoint& screenPos);
	void HidePopup();
	bool IsPopupVisible() const;
	void OffsetSelection(int offset);
	launcherapp::core::Command* GetCurrentCommand() const;

protected:
	DECLARE_MESSAGE_MAP()

private:
	/**
	  Popupの高さと子リストのサイズを現在の候補数・行高に合わせて更新する
	*/
	void ResizePopup();

	std::unique_ptr<ExtraCandidatePopupWnd> mPopupWnd;
	std::vector<RefPtr<launcherapp::core::Command>> mCandidates;
	int mRowHeight{22};
};
