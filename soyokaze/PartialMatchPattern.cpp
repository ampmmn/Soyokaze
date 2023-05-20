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
	std::wregex mRegPatternFront;
	std::wregex mRegPatternPartial;
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

	std::wstring escapedPat = Pattern::StripEscapeChars(pattern);

	std::wstring patFront(L"^");
	patFront += escapedPat;
	in->mRegPatternFront = std::wregex(patFront, std::regex_constants::icase);

	in->mRegPatternPartial = std::wregex(escapedPat, std::regex_constants::icase);
}

int PartialMatchPattern::Match(
	const CString& str
)
{
	if (str.CompareNoCase(in->mWord) == 0) {
		return WholeMatch;
	}
	if (std::regex_search((const wchar_t*)str, in->mRegPatternFront)) {
		return FrontMatch;
	}

	if (std::regex_search((const wchar_t*)str, in->mRegPatternPartial) == false) {
		return Mismatch;
	}
	return PartialMatch;
}

CString PartialMatchPattern::GetOriginalPattern()
{
	return in->mWord;
}

