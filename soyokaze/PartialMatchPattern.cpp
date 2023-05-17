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
	CString mWord;
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
	in->mWord = pattern;
	std::wstring pat(Pattern::StripEscapeChars(pattern));
	in->mRegPattern = std::wregex(pat, std::regex_constants::icase);
}

bool PartialMatchPattern::Match(
	const CString& str
)
{
	return std::regex_search((const wchar_t*)str, in->mRegPattern);
}

CString PartialMatchPattern::GetOriginalPattern()
{
	return in->mWord;
}

