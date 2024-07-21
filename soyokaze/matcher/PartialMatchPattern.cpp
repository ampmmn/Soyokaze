#include "pch.h"
#include "framework.h"
#include "matcher/PartialMatchPattern.h"
#include "setting/AppPreference.h"
#include "matcher/Migemo.h"
#include "commands/core/CommandParameter.h"
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

	// Migemoを使うかどうか
	bool shouldUseMigemo(size_t index, const CString& token)
 	{
		// Migemoが初期化されていないなら使わない(使えない)
		if (mMigemo.IsInitialized() == false) {
			return false;
		}

		// 入力文字が1文字で子音の場合はC/Migemoを使わない
		// (子音によっては何もヒットしないことがあるので)
		static CString vowelChars(_T("aiueoAIUEO"));
		if (token.GetLength() == 1) {
			bool isVowel = (vowelChars.Find(token[0]) != -1);
			if (isVowel == false) {
				return false;
			}
		}

		// 先頭2ワードまではMigemoを使う
		bool is1stOr2ndWord = (index == 0 || index == 1);
		if (is1stOr2ndWord == false) {
		 return false;
		}

		return true;
	};

	// 入力文字列をばらした配列
	std::vector<CString> mTokens;

	std::vector<std::wregex> mRegPatterns;

	// 比較用のデータ(WORDには文字列or正規表現のどちらかが入る)
	std::vector<WORD> mWords;
	// 前方一致用のパターン
	std::vector<std::wregex> mRegPatternsForFM;
	std::wregex mRegPatternFront;

	CString mWholeText;
	bool mIsUseMigemoForHistory;

	Migemo mMigemo;
};

PartialMatchPattern::PartialMatchPattern() : in(std::make_unique<PImpl>())
{
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

static tstring tostring(std::regex_constants::error_type e)
{
  switch (e) {
		case std::regex_constants::error_collate:    return _T("error_collapse");
    case std::regex_constants::error_ctype:      return _T("error_ctype");
    case std::regex_constants::error_escape:     return _T("error_escape");
    case std::regex_constants::error_backref:    return _T("error_back reference");
    case std::regex_constants::error_brack:      return _T("error_bracket");
    case std::regex_constants::error_paren:      return _T("error_paren");
    case std::regex_constants::error_brace:      return _T("error_brace");
    case std::regex_constants::error_badbrace:   return _T("error_bad brace");
    case std::regex_constants::error_range:      return _T("error_range");
    case std::regex_constants::error_space:      return _T("error_space");
    case std::regex_constants::error_badrepeat:  return _T("error_bad repeat");
    case std::regex_constants::error_complexity: return _T("error_complexity");
    case std::regex_constants::error_stack:      return _T("error_stack");
    default:                                     return _T("error_unknown");
  }
}

void PartialMatchPattern::SetParam(
	const launcherapp::core::CommandParameter& param
)
{
	const CString& wholeText = param.GetWholeString();

	in->mWholeText = wholeText;


	std::vector<CString> tokens;

	int start = 0;

	int wholeLen = wholeText.GetLength();

	// tokensの構築
	bool isQuote = false;
	for (int i = 0; i < wholeLen; ++i) {

		TCHAR c = wholeText[i];

		if (isQuote == false && c == _T('"')) {
			isQuote = true;
			start = i+1;
			continue;
		}
		if (isQuote != false && c == _T('"')) {
			isQuote = false;

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

		if (isQuote == false && c == _T(' ')) {

			int count = i-start;

			CString part = wholeText.Mid(start, count);
			part.Trim();
			if (part.IsEmpty()) {
				start=i+1;
				continue;
			}

			// 残り部分が一つの絶対パスを表しているなら、連結してひと固まりとして扱う
			CString partLeftAll(wholeText.Mid(start));
			if (PathIsRelative(partLeftAll) == FALSE && PathFileExists(partLeftAll)) {
				tokens.push_back(partLeftAll);
				break;
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

	for (size_t i = 0; i < in->mTokens.size(); ++i) {
		auto& token = in->mTokens[i];
		ASSERT(token.GetLength() > 0);

		// 2つ目以降のキーワードが絶対パス表記ならパターンから除外(単なるパラメータとしての扱いとし、マッチングには使用しない)
		if (i > 0 && PathIsRelative(token) == FALSE) {
			continue;
		}

		try {
			if (in->shouldUseMigemo(i, token)) {

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
		}
		catch (std::regex_error& e) {

			tstring reason = tostring(e.code());
			spdlog::debug(_T("Failed to build regex from Migemo. reason={0} input={1}"),
			              reason.c_str(), (LPCTSTR)token);

			std::wstring escapedPat = Pattern::StripEscapeChars(token);
			patterns.push_back(std::wregex(escapedPat, std::regex_constants::icase));
			words.push_back(WORD(token, Pattern::FixString));
			break;
		}

		try {
			// 前方一致比較用にパターンを生成しておく
			std::wstring escapedPat = Pattern::StripEscapeChars(in->mTokens[i]);
			patternsForFM.push_back(std::wregex(L"^" + escapedPat));
		}
		catch (std::regex_error& e) {
			tstring reason = tostring(e.code());
			spdlog::warn(_T("Failed to build regex for frontmatch. reason={0} "), reason.c_str());
		}
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

void PartialMatchPattern::GetRawWords(std::vector<CString>& words)
{
	words = in->mTokens;
}

int PartialMatchPattern::GetWordCount()
{
	return (int)in->mWords.size();
}


