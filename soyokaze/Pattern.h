#pragma once

namespace soyokaze {
namespace core {

	class CommandParameter;
}
}


// キーワード比較処理のためのインタフェース
class Pattern
{
public:
	enum MatchLevel {
		WholeMatch = 4,    // 完全一致
		FrontMatch = 3,    // 前方一致
		PartialMatch = 2,  // 部分一致
		Mismatch = -1,     // 不一致
	};

	enum MatchMethod {
		RegExp,           // 正規表現比較
		FixString,        // 文字列比較
	};

	struct WORD {
		WORD(const CString& word, MatchMethod method);

		CString mWord;    // パターン
		MatchMethod mMethod; // 比較方法
	};

public:
	virtual ~Pattern() {}

	virtual void SetParam(const soyokaze::core::CommandParameter& param) = 0;
	virtual int Match(const CString& str) = 0;
	virtual int Match(const CString& str, int offset) = 0;
	virtual CString GetFirstWord() = 0;
	virtual CString GetWholeString() = 0;
	virtual bool shouldWholeMatch() = 0;
	virtual void GetWords(std::vector<WORD>& words) = 0;
	virtual int GetWordCount() = 0;

	static CString StripEscapeChars(const CString& pat);
};


