#pragma once

#include <memory>
#include "CandidateListListenerIF.h"
#include "setting/AppPreferenceListenerIF.h"

class CandidateList;

class CandidateListCtrl : 
	public CListCtrl,
	public CandidateListListenerIF,
	public AppPreferenceListenerIF
{
public:
	CandidateListCtrl();
	virtual ~CandidateListCtrl();

	// 現在選択中のアイテムの領域を取得
	bool GetCurrentItemRect(RECT* rect);
	void SetCandidateList(CandidateList* candidates);

	void InitColumns();
	void UpdateSize(int cx, int cy);

	int GetItemCountInPage();
	void OnMeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

// CandidateListListenerIF
	void OnUpdateSelect(void* sender) override;
	void OnUpdateItems(void* sender) override;
// AppPreferenceListenerIF
	void OnAppFirstBoot() override;
	void OnAppNormalBoot() override;
	void OnAppPreferenceUpdated() override;
	void OnAppExit() override;
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	DECLARE_MESSAGE_MAP()
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

