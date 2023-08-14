#include "pch.h"
#include "framework.h"
#include "PartialMatchPattern.h"
#include "AppPreference.h"
#include "Migemo.h"
#include "core/CommandParameter.h"
#include <vector>
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct PartialMatchPattern::PImpl
{
	std::vector<std::wregex> mRegPatterns;
	std::wregex mRegPatternFront;

	CString mFirstWord;
	CString mWholeText;
	bool mHasError;

	Migemo mMigemo;
};

PartialMatchPattern::PartialMatchPattern() : in(std::make_unique<PImpl>())
{
	in->mHasError = false;

	auto pref = AppPreference::Get();

	if (pref->IsEnableMigemo()) {
		// Migemoの辞書設定
		TCHAR dictPath[MAX_PATH_NTFS];
		GetModuleFileName(nullptr, dictPath, MAX_PATH_NTFS);
		PathRemoveFileSpec(dictPath);
		PathAppend(dictPath, _T("dict\\utf-8\\migemo-dict"));
		if (PathFileExists(dictPath)) {
			in->mMigemo.Open(dictPath);
		}
	}
}

PartialMatchPattern::~PartialMatchPattern()
{
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

	bool is1stWord = true;
	for (auto& word : words) {

		try {

			if (is1stWord && in->mMigemo.IsInitialized()) {
				// Migemoを使う設定の場合、先頭ワードのみMigemo正規表現に置き換える
				CString migemoExpr;
				in->mMigemo.Query(word, migemoExpr);
				patterns.push_back(std::wregex(std::wstring((const wchar_t*)migemoExpr), std::regex_constants::icase));
			}
			else {
				std::wstring escapedPat = Pattern::StripEscapeChars(word);
				patterns.push_back(std::wregex(escapedPat, std::regex_constants::icase));
			}
			is1stWord = false;
		}
		catch (std::regex_error&) {
			in->mHasError = true;
			break;
		}
	}
	in->mRegPatterns.swap(patterns);

	if (words.empty() == false) {
		in->mFirstWord = words[0];

		std::wstring escapedPat = Pattern::StripEscapeChars(words[0]);
		in->mRegPatternFront = std::wregex(L"^" + escapedPat);
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

	// 入力されたキーワードに完全一致するかどうかの判断
	if (str.CompareNoCase(in->mFirstWord) == 0) {
		return WholeMatch;
	}



	for (auto& pat : in->mRegPatterns) {

		// ひとつでもマッチしないものがあったら、ヒットしないものとみなす
		if (std::regex_search((const wchar_t*)str, pat) == false) {
			return Mismatch;
		}
	}

	// 先頭のキーワードに前方一致する場合は前方一致とみなす
	if (std::regex_search((const wchar_t*)str, in->mRegPatternFront)) {
		return FrontMatch;
	}

	// そうでなければ部分一致
	return PartialMatch;
}

CString PartialMatchPattern::GetFirstWord()
{
	return in->mFirstWord;
}

CString PartialMatchPattern::GetWholeString()
{
	return in->mWholeText;
}

