#pragma once

class CandidateListRenderer
{
public:
	virtual ~CandidateListRenderer() {}

	virtual void DrawItem(CWnd* listWnd, LPDRAWITEMSTRUCT drawItemStruct) = 0;
	virtual void UpdateSize(int cx, int cy) = 0;
	virtual void SetIsEmpty(bool isEmpty) = 0;
	virtual int GetItemCountInPage() const = 0;
};
