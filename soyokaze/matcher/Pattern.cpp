#include "pch.h"
#include "framework.h"
#include "Pattern.h"


Pattern::WORD::WORD(const CString& word, MatchMethod method) :
 	mWord(word), mMethod(method)
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CString Pattern::StripEscapeChars(const CString& pattern)
{
	// 後段のwregexに値を渡したときにエスケープ記号として解釈されるのを防ぐため

	CString tmp(pattern);
	tmp.Replace(_T("\\"), _T("\\\\"));
	tmp.Replace(_T("?"), _T("\\?"));
	tmp.Replace(_T("."), _T("\\."));
	tmp.Replace(_T("*"), _T("\\*"));
	tmp.Replace(_T("+"), _T("\\+"));
	tmp.Replace(_T("^"), _T("\\^"));
	tmp.Replace(_T("$"), _T("\\$"));
	tmp.Replace(_T("["), _T("\\["));
	tmp.Replace(_T("]"), _T("\\]"));
	tmp.Replace(_T("|"), _T("\\|"));
	tmp.Replace(_T("("), _T("\\("));
	tmp.Replace(_T(")"), _T("\\)"));
	tmp.Replace(_T("{"), _T("\\{"));
	tmp.Replace(_T("}"), _T("\\}"));
	tmp.Replace(_T("!"), _T("\\!"));
	tmp.Replace(_T("&"), _T("\\&"));

	return tmp;
}

