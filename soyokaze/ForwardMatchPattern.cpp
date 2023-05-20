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
	CString mWord;
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
	in->mWord = pattern;

	std::wstring pat(L"^");
	pat += Pattern::StripEscapeChars(pattern);
	in->mRegPattern = std::wregex(pat, std::regex_constants::icase);
}

int ForwardMatchPattern::Match(
	const CString& str
)
{
	if (str.CompareNoCase(in->mWord) == 0) {
		return WholeMatch;
	}

	if (std::regex_search((const wchar_t*)str, in->mRegPattern) == false) {
		return Mismatch;
	}

	return FrontMatch;
}


CString ForwardMatchPattern::GetOriginalPattern()
{
	return in->mWord;
}

