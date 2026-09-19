#pragma once

#include "StandardCandidateListRenderer.h"

class BGImageCandidateListRenderer : public StandardCandidateListRenderer
{
public:
	BGImageCandidateListRenderer();
	~BGImageCandidateListRenderer() override;

	/**
	  候補欄の背景色を交互に描画する設定を更新する
	  @param[in] isAlternateColor 交互色を使用する場合はtrue
	*/
	void SetIsAlternateColor(bool isAlternateColor) override;
	/**
	  候補項目と背景画像を描画する
	  @param[in] listWnd 候補欄のウインドウ
	  @param[in] drawItemStruct 描画対象項目の情報
	*/
	void DrawItem(CWnd* listWnd, LPDRAWITEMSTRUCT drawItemStruct) override;
	/**
	  背景画像を描画する領域のサイズを更新する
	  @param[in] cx 描画領域の幅
	  @param[in] cy 描画領域の高さ
	*/
	void UpdateSize(int cx, int cy) override;
	/**
	  候補欄が空かどうかを更新する
	  @param[in] isEmpty 候補欄が空の場合はtrue
	*/
	void SetIsEmpty(bool isEmpty) override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
