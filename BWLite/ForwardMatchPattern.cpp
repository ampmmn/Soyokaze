#include "pch.h"
#include "framework.h"
#include "ForwardMatchPattern.h"
#include <vector>
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct ForwardMatchPattern::PImpl
{
	std::wregex mRegPattern;
};

ForwardMatchPattern::ForwardMatchPattern() : in(new PImpl)
{
}

ForwardMatchPattern::~ForwardMatchPattern()
{
	delete in;
}

void ForwardMatchPattern::SetPattern(
	const CString& pattern
)
{
	// 後段のwregexに値を渡したときにエスケープ記号として解釈されるのを防ぐため'\'を付与する
	std::wstring pat(L"^");
	CString tmp(pattern);

	tmp.Replace(_T("\\"), _T("\\\\"));
	pat += tmp;
	in->mRegPattern = std::wregex(pat, std::regex_constants::icase);
}

bool ForwardMatchPattern::Match(
	const CString& str
)
{
	return std::regex_search((const wchar_t*)str, in->mRegPattern);
}


