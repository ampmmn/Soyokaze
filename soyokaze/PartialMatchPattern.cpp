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
	bool GetKeyword(int index, CString& word)
	{
		if (index < 0 || mTokens.size() <= (size_t)index) {
			return false;
		}
		word = mTokens[index];
		return true;
	}

	// 入力文字列をばらした配列
	std::vector<CString> mTokens;

	std::vector<std::wregex> mRegPatterns;

	// 比較用のデータ(WORDには文字列or正規表現のどちらかが入る)
	std::vector<WORD> mWords;
	// 前方一致用のパターン
	std::vector<std::wregex> mRegPatternsForFM;
	std::wregex mRegPatternFront;

	CString mWholeText;
	bool mHasError;
	bool mIsUseMigemoForHistory;

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
	in->mIsUseMigemoForHistory = pref->IsUseMigemoForBrowserHistory();
}

PartialMatchPattern::~PartialMatchPattern()
{
}

static bool IsVowel(TCHAR c)
{
	static CString vowelChars(_T("aiueoAIUEO"));
	return vowelChars.Find(c) != -1;
}

void PartialMatchPattern::SetParam(
	const soyokaze::core::CommandParameter& param
)
{
	const CString& wholeText = param.GetWholeString();

	in->mHasError = false;
	in->mWholeText = wholeText;


	std::vector<CString> tokens;

	int start = 0;

	int wholeLen = wholeText.GetLength();

	// tokensの構築
	bool isQuate = false;
	for (int i = 0; i < wholeLen; ++i) {

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

			tokens.push_back(part);
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

			tokens.push_back(part);

			start=i+1;
			continue;
		}
	}

	int count = wholeLen-start;
	CString part = wholeText.Mid(start, count);
	part.Trim();
	if (part.IsEmpty() == FALSE) {
		tokens.push_back(part);
	}

	in->mTokens.swap(tokens);

	std::vector<WORD> words;
	std::vector<std::wregex> patterns;
	std::vector<std::wregex> patternsForFM;
	patterns.reserve(in->mTokens.size());

	bool is1stWord = true;  // 先頭のワードか?

	for (auto& token : in->mTokens) {
		ASSERT(token.GetLength() > 0);

		try {
			// 入力文字が1文字で子音の場合はC/Migemoを使わない
			// (子音によっては何もヒットしないことがあるので)
			if (is1stWord && in->mMigemo.IsInitialized() && (token.GetLength() > 1 || IsVowel(token[0])) ) {

				// Migemoを使う設定の場合、先頭ワードのみMigemo正規表現に置き換える
				CString migemoExpr;
				in->mMigemo.Query(token, migemoExpr);
				patterns.push_back(std::wregex(std::wstring((const wchar_t*)migemoExpr), std::regex_constants::icase));

				// ↓ChromiumBrowseHistory用のデータ。
				if (in->mIsUseMigemoForHistory) {
					words.push_back(WORD(migemoExpr, Pattern::RegExp));
				}
				else {
					words.push_back(WORD(token, Pattern::FixString));
				}
			}
			else {
				std::wstring escapedPat = Pattern::StripEscapeChars(token);
				patterns.push_back(std::wregex(escapedPat, std::regex_constants::icase));

				words.push_back(WORD(token, Pattern::FixString));
			}
			is1stWord = false;
		}
		catch (std::regex_error&) {
			in->mHasError = true;
			break;
		}

		// 前方一致比較用にパターンを生成しておく
		std::wstring escapedPat = Pattern::StripEscapeChars(in->mTokens[0]);
		patternsForFM.push_back(std::wregex(L"^" + escapedPat));
	}
	in->mRegPatterns.swap(patterns);
	in->mRegPatternsForFM.swap(patternsForFM);
	in->mWords.swap(words);
}

int PartialMatchPattern::Match(
	const CString& str
)
{
	return Match(str, 0);
}

int PartialMatchPattern::Match(
	const CString& str,
	int offset
)
{
	if (in->mHasError) {
		// エラー時は無効化
		return Mismatch;
	}

	// 入力されたキーワードに完全一致するかどうかの判断
	CString keyword;
	if (in->GetKeyword(offset, keyword) &&
	    str.CompareNoCase(keyword) == 0) {
		return WholeMatch;
	}

	size_t regPatCount = in->mRegPatterns.size();
	for (size_t i = offset; i < regPatCount; ++i) {
		auto& pat = in->mRegPatterns[i];

		// ひとつでもマッチしないものがあったら、ヒットしないものとみなす
		if (std::regex_search((const wchar_t*)str, pat) == false) {
			return Mismatch;
		}
	}

	// 先頭のキーワードに前方一致する場合は前方一致とみなす
	if (offset < in->mRegPatternsForFM.size()) {
		auto& patForFM = in->mRegPatternsForFM[offset];
		if (std::regex_search((const wchar_t*)str, patForFM)) {
			return FrontMatch;
		}
	}

	// そうでなければ部分一致
	return PartialMatch;
}

CString PartialMatchPattern::GetFirstWord()
{
	return in->mTokens.empty() ? _T("") : in->mTokens[0];
}

CString PartialMatchPattern::GetWholeString()
{
	return in->mWholeText;
}

bool PartialMatchPattern::shouldWholeMatch()
{
	return false;
}

void PartialMatchPattern::GetWords(std::vector<WORD>& words)
{
	words = in->mWords;
}

int PartialMatchPattern::GetWordCount()
{
	return (int)in->mWords.size();
}


