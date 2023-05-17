#pragma once

// キーワード比較処理のためのインタフェース
class Pattern
{
public:
	virtual ~Pattern() {}

	virtual void SetPattern(const CString& pattern) = 0;
	virtual bool Match(const CString& str) = 0;
	virtual CString GetOriginalPattern() = 0;

	static CString StripEscapeChars(const CString& pat);
};


