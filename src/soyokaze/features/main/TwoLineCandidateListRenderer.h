#pragma once

#include "StandardCandidateListRenderer.h"

class TwoLineCandidateListRenderer : public StandardCandidateListRenderer
{
public:
	/**
	  コマンド名と説明を二行表示するレンダラーを生成する
	*/
	TwoLineCandidateListRenderer();
	~TwoLineCandidateListRenderer() override;

	/**
	  二行表示での候補項目の高さを取得する
	  @return 候補項目の高さ
	*/
	int GetItemHeight() const override;

protected:
	/**
	  コマンド名と説明を二行で描画する
	  @param[in] listWnd 候補欄
	  @param[in] pDC 描画先デバイスコンテキスト
	  @param[in] itemId 項目番号
	*/
	void DrawItemName(CListCtrl* listWnd, CDC* pDC, int itemId) override;
	/**
	  コマンド種別を一行目の位置に描画する
	  @param[in] listWnd 候補欄
	  @param[in] pDC 描画先デバイスコンテキスト
	  @param[in] itemId 項目番号
	*/
	void DrawItemCategory(CListCtrl* listWnd, CDC* pDC, int itemId) override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
