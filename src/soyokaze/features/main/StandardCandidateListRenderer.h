#pragma once

#include "CandidateListRenderer.h"

class CandidateList;

class StandardCandidateListRenderer : public CandidateListRenderer
{
public:
	StandardCandidateListRenderer();
	~StandardCandidateListRenderer() override;

	void SetCandidateList(CandidateList* candidates) override;
	/**
	  候補欄の背景色を交互に描画する設定を更新する
	  @param[in] isAlternateColor 交互色を使用する場合はtrue
	*/
	void SetIsAlternateColor(bool isAlternateColor) override;
	void SetIsShowCommandType(bool isShowCommandType) override;
	void SetIsDrawIcon(bool isDrawIcon) override;
	void SetIsDrawBackground(bool isDrawBackground) override;
	void SetTextMetrics(int textHeight, int textLineHeight, int iconSize) override;
	CImageList* GetImageList() override;

	void DrawItem(CWnd* listWnd, LPDRAWITEMSTRUCT drawItemStruct) override;
	void UpdateSize(int cx, int cy) override;
	void SetIsEmpty(bool isEmpty) override;
	int GetItemCountInPage() const override;
	/**
	  表示形式に応じた候補項目の高さを取得する
	  @return 候補項目の高さ
	*/
	int GetItemHeight() const override;

protected:
	/**
	  項目名を二行表示するかどうかを設定する
	  @param[in] isTwoLine 二行表示する場合はtrue
	*/
	void SetIsTwoLine(bool isTwoLine);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
