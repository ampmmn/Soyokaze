#pragma once

#include "CandidateListRenderer.h"

class BGImageCandidateListRenderer : public CandidateListRenderer
{
public:
	explicit BGImageCandidateListRenderer(std::unique_ptr<CandidateListRenderer> renderer);
	~BGImageCandidateListRenderer() override;

	/**
	  描画対象の候補リストを設定する
	  @param[in] candidates 候補リスト
	*/
	void SetCandidateList(CandidateList* candidates) override;
	/**
	  候補欄の背景色を交互に描画する設定を更新する
	  @param[in] isAlternateColor 交互色を使用する場合はtrue
	*/
	void SetIsAlternateColor(bool isAlternateColor) override;
	/**
	  コマンド種別を表示するかどうかを設定する
	  @param[in] isShowCommandType 表示する場合はtrue
	*/
	void SetIsShowCommandType(bool isShowCommandType) override;
	/**
	  アイコンを描画するかどうかを設定する
	  @param[in] isDrawIcon 描画する場合はtrue
	*/
	void SetIsDrawIcon(bool isDrawIcon) override;
	/**
	  背景色を描画するかどうかを設定する
	  @param[in] isDrawBackground 描画する場合はtrue
	*/
	void SetIsDrawBackground(bool isDrawBackground) override;
	/**
	  テキスト寸法とアイコンサイズを設定する
	  @param[in] textHeight テキストの高さ
	  @param[in] textLineHeight 行送りを含むテキストの高さ
	  @param[in] iconSize アイコンのサイズ
	*/
	void SetTextMetrics(int textHeight, int textLineHeight, int iconSize) override;
	/**
	  リストコントロールに設定する画像リストを取得する
	  @return 描画に使用する画像リスト
	*/
	CImageList* GetImageList() override;

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
	/**
	  1ページ内に表示できる項目数を取得する
	  @return ページ内の項目数
	*/
	int GetItemCountInPage() const override;
	/**
	  内部レンダラーが必要とする候補項目の高さを取得する
	  @return 候補項目の高さ
	*/
	int GetItemHeight() const override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
