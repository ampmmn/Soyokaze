#include "pch.h"
#include "framework.h"
#include "SkipMatchPattern.h"
#include <vector>
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct SkipMatchPattern::PImpl
{
	std::wregex mRegPattern;
};

SkipMatchPattern::SkipMatchPattern() : in(new PImpl)
{
}

SkipMatchPattern::~SkipMatchPattern()
{
	delete in;
}

void SkipMatchPattern::SetPattern(
	const CString& pattern
)
{
	// 1文字ごとに".*"を付けたうえで正規表現マッチングをすることにより、
	// スキップマッチング的動作を実現する

	std::wstring pat;
	for (int i = 0; i < pattern.GetLength(); ++i) {
		pat += pattern[i];

		if (pattern[i] == _T('\\')) {
			// 後段のwregexに値を渡したときにエスケープ記号として解釈されるのを防ぐため'\'を付与する
			pat += pattern[i];
			continue;
		}


		if (0xD800 <= pattern[i] && pattern[i] <= 0xDBFF) {
			// サロゲートペアの先頭1byteは後続の2バイト目と不可分のため、
			// 「.*」をつけない
			continue;
		}

		pat += _T(".*");
	}
	in->mRegPattern = std::wregex(pat, std::regex_constants::icase);
}

bool SkipMatchPattern::Match(
	const CString& str
)
{
	return std::regex_search((const wchar_t*)str, in->mRegPattern);
}


