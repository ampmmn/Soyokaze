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
	  一行表示での候補項目の高さを取得する
	  @return 候補項目の高さ
	*/
	int GetItemHeight() const override;

protected:
	/**
	  項目名を描画する
	  @param[in] listWnd 候補欄
	  @param[in] pDC 描画先デバイスコンテキスト
	  @param[in] itemId 項目番号
	*/
	virtual void DrawItemName(CListCtrl* listWnd, CDC* pDC, int itemId);
	/**
	  コマンド種別を描画する
	  @param[in] listWnd 候補欄
	  @param[in] pDC 描画先デバイスコンテキスト
	  @param[in] itemId 項目番号
	*/
	virtual void DrawItemCategory(CListCtrl* listWnd, CDC* pDC, int itemId);
	/**
	  描画対象の候補リストを取得する
	  @return 候補リスト
	*/
	CandidateList* GetCandidateList() const;
	/**
	  テキストの高さを取得する
	  @return テキストの高さ
	*/
	int GetTextHeight() const;
	/**
	  コマンド種別を表示する設定かどうかを取得する
	  @return 表示する場合はtrue
	*/
	bool IsShowCommandType() const;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
