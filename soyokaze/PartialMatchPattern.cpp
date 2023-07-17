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
	std::vector<std::wregex> mRegPatterns;
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
	const CString& wholeText = param.GetWholeString();

	in->mHasError = false;
	in->mWholeText = wholeText;


	std::vector<CString> words;

	int start = 0;

	int len = wholeText.GetLength();

	bool isQuate = false;
	for (int i = 0; i < len; ++i) {

		TCHAR c = wholeText[i];

		if (isQuate == false && c == _T('"')) {
			isQuate = true;
			start = i+1;
			continue;
		}
		if (isQuate != false && c == _T('"')) {
			isQuate = false;

			int count = i - start;
			CString part = wholeText.Mid(start, count);
			if (part.IsEmpty()) {
				start=i+1;
				continue;
			}

			words.push_back(part);
			start = i + 1;
			continue;
		}

		if (isQuate == false && c == _T(' ')) {

			int count = i-start;

			CString part = wholeText.Mid(start, count);
			part.Trim();
			if (part.IsEmpty()) {
				start=i+1;
				continue;
			}

			words.push_back(part);

			start=i+1;
			continue;
		}
	}

	int count = len-start;
	CString part = wholeText.Mid(start, count);
	part.Trim();
	if (part.IsEmpty() == FALSE) {
		words.push_back(part);
	}


	std::vector<std::wregex> patterns;
	patterns.reserve(words.size());

	for (auto& word : words) {

		try {
			std::wstring escapedPat = Pattern::StripEscapeChars(word);
			patterns.push_back(std::wregex(escapedPat, std::regex_constants::icase));
		}
		catch (std::regex_error&) {
			in->mHasError = true;
			break;
		}
	}
	in->mRegPatterns.swap(patterns);

	if (words.empty() == false) {
		in->mWord = words[0];
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

	for (auto& pat : in->mRegPatterns) {

		// ひとつでもマッチしないものがあったら、ヒットしないものとみなす
		if (std::regex_search((const wchar_t*)str, pat) == false) {
			return Mismatch;
		}

	}
	return PartialMatch;
}

CString PartialMatchPattern::GetOriginalPattern()
{
	return in->mWholeText;
}

