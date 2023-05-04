#include "pch.h"
#include "framework.h"
#include "Pattern.h"

CString Pattern::StripEscapeChars(const CString& pattern)
{
	// 後段のwregexに値を渡したときにエスケープ記号として解釈されるのを防ぐため

	CString tmp(pattern);
	tmp.Replace(_T("\\"), _T(""));
	tmp.Replace(_T("?"), _T(""));
	tmp.Replace(_T("."), _T(""));
	tmp.Replace(_T("+"), _T(""));
	tmp.Replace(_T("^"), _T(""));
	tmp.Replace(_T("$"), _T(""));
	tmp.Replace(_T("["), _T(""));
	tmp.Replace(_T("]"), _T(""));
	tmp.Replace(_T("|"), _T(""));
	tmp.Replace(_T("("), _T(""));
	tmp.Replace(_T(")"), _T(""));
	tmp.Replace(_T("{"), _T(""));
	tmp.Replace(_T("}"), _T(""));
	tmp.Replace(_T("!"), _T(""));
	tmp.Replace(_T("&"), _T(""));

	return tmp;
}

