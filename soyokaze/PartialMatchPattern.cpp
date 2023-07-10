#include "pch.h"
#include "framework.h"
#include "PartialMatchPattern.h"
#include "core/CommandParameter.h"
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
	CString mWholeText;
	bool mHasError;
};

PartialMatchPattern::PartialMatchPattern() : in(new PImpl)
{
	in->mHasError = false;
}

PartialMatchPattern::~PartialMatchPattern()
{
	delete in;
}

void PartialMatchPattern::SetParam(
	const soyokaze::core::CommandParameter& param
)
{
	const CString& pattern = param.GetCommandString();

	in->mHasError = false;
	in->mWord = pattern;
	in->mWholeText = param.GetWholeString();

	std::wstring escapedPat = Pattern::StripEscapeChars(pattern);

	std::wstring patFront(L"^");
	patFront += escapedPat;

	try {
		in->mRegPatternFront = std::wregex(patFront, std::regex_constants::icase);

		in->mRegPatternPartial = std::wregex(escapedPat, std::regex_constants::icase);

	}
	catch (std::regex_error&) {
		in->mHasError = true;
	}
}

int PartialMatchPattern::Match(
	const CString& str
)
{
	if (in->mHasError) {
		// エラー時は無効化
		return Mismatch;
	}

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
	return in->mWholeText;
}

