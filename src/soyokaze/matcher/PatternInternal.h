#pragma once

#include "matcher/Pattern.h"
#include <vector>

// キーワード比較処理のためのインタフェース(内部用の追加機能)
class PatternInternal : virtual public Pattern
{
public:
	enum MatchMethod {
		RegExp,           // 正規表現比較
		FixString,        // 文字列比較
	};

	struct WORD {
		WORD(const CString& word, const CString& rawWords, MatchMethod method);
		WORD(const CString& word);

		CString mWord;       // パターン
		CString mRawWord;    // 元の文字
		MatchMethod mMethod; // 比較方法
	};


public:
	virtual void GetWords(std::vector<WORD>& words) = 0;
	virtual void GetRawWords(std::vector<CString>& words) = 0;
};


