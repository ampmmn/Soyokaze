#include "pch.h"
#include "framework.h"
#include "SkipMatchPattern.h"
#include "core/CommandParameter.h"
#include <vector>
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct SkipMatchPattern::PImpl
{
	std::wregex mRegPatternFront;
	std::wregex mRegPatternPartial;
	std::wregex mRegPatternSkip;
	CString mWord;
	CString mWholeText;
	bool mHasError;
};

SkipMatchPattern::SkipMatchPattern() : in(new PImpl)
{
	in->mHasError = false;
}

SkipMatchPattern::~SkipMatchPattern()
{
	delete in;
}

void SkipMatchPattern::SetParam(
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

		// 1文字ごとに".*"を付けたうえで正規表現マッチングをすることにより、
		// スキップマッチング的動作を実現する
		CString stripped = Pattern::StripEscapeChars(pattern);

		std::wstring pat;
		for (int i = 0; i < escapedPat.size(); ++i) {
			pat += escapedPat[i];

			if (stripped[i] == _T('\\')) {
				// 後段のwregexに値を渡したときにエスケープ記号として解釈されるのを防ぐため'\'を付与する
				pat += stripped[i];
				continue;
			}


			if (0xD800 <= stripped[i] && stripped[i] <= 0xDBFF) {
				// サロゲートペアの先頭1byteは後続の2バイト目と不可分のため、
				// 「.*」をつけない
				continue;
			}

			pat += _T(".*");
		}
		in->mRegPatternSkip = std::wregex(pat, std::regex_constants::icase);
	}
	catch (std::regex_error&) {
		in->mHasError = true;
	}
}

int SkipMatchPattern::Match(
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

	if (std::regex_search((const wchar_t*)str, in->mRegPatternPartial)) {
		return PartialMatch;
	}
	if (std::regex_search((const wchar_t*)str, in->mRegPatternSkip)) {
		return SkipMatch;
	}
	return Mismatch;
}

CString SkipMatchPattern::GetOriginalPattern()
{
	return in->mWholeText;
}

