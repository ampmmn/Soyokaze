#pragma once

#include "CandidateListRenderer.h"

class CandidateList;

class StandardCandidateListRenderer : public CandidateListRenderer
{
public:
	StandardCandidateListRenderer();
	~StandardCandidateListRenderer() override;

	void SetCandidateList(CandidateList* candidates);
	void SetIsAlternateColor(bool isAlternateColor);
	void SetIsShowCommandType(bool isShowCommandType);
	void SetIsDrawIcon(bool isDrawIcon);
	void SetTextMetrics(int textHeight, int iconSize);
	CImageList* GetImageList();

	void DrawItem(CWnd* listWnd, LPDRAWITEMSTRUCT drawItemStruct) override;
	void UpdateSize(int cx, int cy) override;
	void SetIsEmpty(bool isEmpty) override;
	int GetItemCountInPage() const override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
