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
		SkipMatch = 1,     // スキップマッチ
		Mismatch = -1,     // 不一致
	};

public:
	virtual ~Pattern() {}

	virtual void SetParam(const soyokaze::core::CommandParameter& param) = 0;
	virtual int Match(const CString& str) = 0;
	virtual CString GetOriginalPattern() = 0;

	static CString StripEscapeChars(const CString& pat);
};


