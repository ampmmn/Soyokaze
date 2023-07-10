#include "pch.h"
#include "framework.h"
#include "ForwardMatchPattern.h"
#include "core/CommandParameter.h"
#include <vector>
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct ForwardMatchPattern::PImpl
{
	tregex mRegPattern;
	CString mWord;
	CString mWholeText;
	bool mHasError;
};

ForwardMatchPattern::ForwardMatchPattern() : in(new PImpl)
{
	in->mHasError = false;
}

ForwardMatchPattern::~ForwardMatchPattern()
{
	delete in;
}

void ForwardMatchPattern::SetParam(const soyokaze::core::CommandParameter& param)
{
	const CString& pattern = param.GetCommandString();

	in->mHasError = false;
	in->mWord = pattern;
	in->mWholeText = param.GetWholeString();

	tstring pat(_T("^"));
	pat += Pattern::StripEscapeChars(pattern);

	try {
		in->mRegPattern = tregex(pat, std::regex_constants::icase);
	}
	catch (std::regex_error&) {
		in->mHasError = true;
	}
}


int ForwardMatchPattern::Match(
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

	if (std::regex_search((const wchar_t*)str, in->mRegPattern) == false) {
		return Mismatch;
	}

	return FrontMatch;
}


CString ForwardMatchPattern::GetOriginalPattern()
{
	return in->mWholeText;
}

