#pragma once

class CandidateList;

class CandidateListRenderer
{
public:
	virtual ~CandidateListRenderer() = default;

	/**
	  描画対象の候補リストを設定する
	  @param[in] candidates 候補リスト
	*/
	virtual void SetCandidateList(CandidateList* candidates) = 0;
	/**
	  候補欄の背景色を交互に描画する設定を更新する
	  @param[in] isAlternateColor 交互色を使用する場合はtrue
	*/
	virtual void SetIsAlternateColor(bool isAlternateColor) = 0;
	/**
	  コマンド種別を表示するかどうかを設定する
	  @param[in] isShowCommandType 表示する場合はtrue
	*/
	virtual void SetIsShowCommandType(bool isShowCommandType) = 0;
	/**
	  アイコンを描画するかどうかを設定する
	  @param[in] isDrawIcon 描画する場合はtrue
	*/
	virtual void SetIsDrawIcon(bool isDrawIcon) = 0;
	/**
	  背景色を描画するかどうかを設定する
	  @param[in] isDrawBackground 描画する場合はtrue
	*/
	virtual void SetIsDrawBackground(bool isDrawBackground) = 0;
	/**
	  テキスト寸法とアイコンサイズを設定する
	  @param[in] textHeight テキストの高さ
	  @param[in] textLineHeight 行送りを含むテキストの高さ
	  @param[in] iconSize アイコンのサイズ
	*/
	virtual void SetTextMetrics(int textHeight, int textLineHeight, int iconSize) = 0;
	/**
	  リストコントロールに設定する画像リストを取得する
	  @return 描画に使用する画像リスト
	*/
	virtual CImageList* GetImageList() = 0;

	virtual void DrawItem(CWnd* listWnd, LPDRAWITEMSTRUCT drawItemStruct) = 0;
	virtual void UpdateSize(int cx, int cy) = 0;
	virtual void SetIsEmpty(bool isEmpty) = 0;
	virtual int GetItemCountInPage() const = 0;
	/**
	  候補項目の高さを取得する
	  @return 候補項目の高さ
	*/
	virtual int GetItemHeight() const = 0;
};
