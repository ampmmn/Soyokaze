#include "pch.h"
#include "framework.h"
#include "PartialMatchPattern.h"
#include <vector>
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct PartialMatchPattern::PImpl
{
	std::wregex mRegPattern;
};

PartialMatchPattern::PartialMatchPattern() : in(new PImpl)
{
}

PartialMatchPattern::~PartialMatchPattern()
{
	delete in;
}

void PartialMatchPattern::SetPattern(
	const CString& pattern
)
{
	// 後段のwregexに値を渡したときにエスケープ記号として解釈されるのを防ぐため'\'を付与する
	CString tmp(pattern);
	tmp.Replace(_T("\\"), _T("\\\\"));

	std::wstring pat(tmp);
	in->mRegPattern = std::wregex(pat, std::regex_constants::icase);
}

bool PartialMatchPattern::Match(
	const CString& str
)
{
	return std::regex_search((const wchar_t*)str, in->mRegPattern);
}


