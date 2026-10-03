#pragma once

#include <memory>

/**
  入力欄のUndo履歴を管理する
*/
class KeywordEditUndoHistory
{
public:
	/** 履歴管理オブジェクトを生成する */
	KeywordEditUndoHistory();
	/** 履歴管理オブジェクトを破棄する */
	~KeywordEditUndoHistory();

	/**
	  履歴の基準となる文字列を設定し、既存の履歴を破棄する
	  @param[in] text    基準となる文字列
	*/
	void Reset(const CString& text);

	/**
	  変更前の文字列を履歴に登録する
	  @return 文字列が変更され、履歴に登録した場合はtrue
	  @param[in] text    変更後の文字列
	*/
	bool RecordChange(const CString& text);

	/**
	  直前の文字列を履歴から復元する
	  @return 復元する履歴がある場合はtrue
	  @param[out] text    復元する文字列
	*/
	bool Undo(CString& text);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
