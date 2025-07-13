#include "pch.h"
#include "framework.h"
#include "matcher/PartialMatchPattern.h"
#include "setting/AppPreference.h"
#include "matcher/Migemo.h"
#include "commands/core/CommandParameter.h"
#include "commands/core/IFIDDefine.h"
#include "utility/Path.h"
#include <vector>
#include <regex>
#include <re2/re2.h>

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
	bool shouldUseMigemo(const CString& token)
 	{
		// Migemoが初期化されていないなら使わない(使えない)
		if (mMigemo.IsInitialized() == false) {
			return false;
		}
		// 文字列が空なら使わない
		if (token.IsEmpty()) {
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
		return true;
	};

	// 入力文字列をばらした配列
	std::vector<CString> mTokens;

	std::vector<RE2*> mRegPatterns;

	// 比較用のデータ(WORDには文字列or正規表現のどちらかが入る)
	std::vector<WORD> mWords;
	// 前方一致用のパターン
	std::vector<RE2*> mRegPatternsForFM;

	CString mWholeText;
	bool mIsUseMigemoForHistory{false};

	Migemo mMigemo;

	uint32_t mRefCount{1};
};

PartialMatchPattern::PartialMatchPattern() : in(std::make_unique<PImpl>())
{
	auto pref = AppPreference::Get();

	if (pref->IsEnableMigemo()) {
		// Migemoの辞書設定
		Path dictPath(Path::MODULEFILEDIR, _T("dict\\utf-8\\migemo-dict"));
		if (dictPath.FileExists()) {
			in->mMigemo.Open(dictPath);
		}
	}
	in->mIsUseMigemoForHistory = pref->IsUseMigemoForBrowserHistory();
}

PartialMatchPattern::~PartialMatchPattern()
{
	for (auto pat : in->mRegPatterns) {
		delete pat;
	}
	in->mRegPatterns.clear();
	for (auto pat : in->mRegPatternsForFM) {
		delete pat;
	}
	in->mRegPatternsForFM.clear();
}

PartialMatchPattern* PartialMatchPattern::Create()
{
	return new PartialMatchPattern();
}

bool PartialMatchPattern::QueryInterface(const IFID& ifid, void** obj)
{
	if (ifid == IFID_PATTERN) {
		AddRef();
		*obj = (Pattern*)this;
		return true;
	}
	if (ifid == IFID_PATTERNINTERNAL) {
		AddRef();
		*obj = (PatternInternal*)this;
		return true;
	}
	return false;
}

uint32_t PartialMatchPattern::AddRef()
{
	return (uint32_t)InterlockedIncrement(&in->mRefCount);
}

uint32_t PartialMatchPattern::Release()
{
	auto n = InterlockedDecrement(&in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return (uint32_t)n;
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

void PartialMatchPattern::SetWholeText(LPCTSTR text)
{
	in->mWholeText = text;
	const CString& wholeText = in->mWholeText;

	// 与えられたテキストを空白で区切ったリスト
	std::vector<CString> tokens;

	int start = 0;

	int wholeLen = wholeText.GetLength();

	// tokensの構築
	bool isQuote = false;
	for (int i = 0; i < wholeLen; ++i) {

		TCHAR c = wholeText[i];

		// "の中であることを識別するためのフラグを立てる
		if (isQuote == false && c == _T('"')) {
			isQuote = true;
			start = i+1;
			continue;
		}

		// "の終わり
		if (isQuote != false && c == _T('"')) {
			isQuote = false;

			// "..." のなかのテキストを取得する
			int count = i - start;
			CString part = wholeText.Mid(start, count);

			tokens.push_back(part);
			start = i + 1;
			continue;
		}

		// "..."でない空白が現れた場合
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
			if (PathIsRelative(partLeftAll) == FALSE && Path::FileExists(partLeftAll)) {
				tokens.push_back(partLeftAll);
				break;
			}

			tokens.push_back(part);

			start=i+1;
			continue;
		}
	}

	// 終端に達した際の末尾にあったキーワードをtokensに追加する
	int count = wholeLen-start;
	CString part = wholeText.Mid(start, count);
	part.Trim();
	if (part.IsEmpty() == FALSE) {
		tokens.push_back(part);
	}

	in->mTokens.swap(tokens);

	std::vector<WORD> words;
	std::vector<RE2*> patterns;
	patterns.reserve(in->mTokens.size());
	std::vector<RE2*> patternsForFM;
	patternsForFM.reserve(in->mTokens.size());

	RE2::Options options;
	options.set_case_sensitive(false);

	// tokensに分解したキーワードをregexに変換してpatternsに入れる
	for (size_t i = 0; i < in->mTokens.size(); ++i) {

		auto& token = in->mTokens[i];

		// 2つ目以降のキーワードが絶対パス表記ならパターンから除外(単なるパラメータとしての扱いとし、マッチングには使用しない)
		if (i > 0 && PathIsRelative(token) == FALSE) {
			continue;
		}

		std::string tmp;

		if (in->shouldUseMigemo(token) == false) {
			if (token.IsEmpty() == FALSE) {
				std::wstring escapedPat = StripEscapeChars(token);
				patterns.push_back(new RE2(UTF2UTF(escapedPat, tmp), options));
				patternsForFM.push_back(new RE2(UTF2UTF(_T("^") + escapedPat, tmp), options));
			}
			words.push_back(WORD(token));
			continue;
		}

		// Migemoを使う設定の場合、先頭ワードのみMigemo正規表現に置き換える
		CString migemoExpr;
		in->mMigemo.Query(token, migemoExpr);
		patterns.push_back(new RE2(UTF2UTF(migemoExpr, tmp), options));

		// ↓ChromiumBrowseHistory用のデータ。
		if (in->mIsUseMigemoForHistory) {
			words.push_back(WORD(migemoExpr, token, PatternInternal::RegExp));
		}
		else {
			words.push_back(WORD(token));
		}

		// 前方一致比較用にパターンを生成しておく
		patternsForFM.push_back(new RE2(UTF2UTF(_T("^") + migemoExpr, tmp), options));
	}
	in->mRegPatterns.swap(patterns);
	for (auto pat : patterns) {
		delete pat;
	}
	in->mRegPatternsForFM.swap(patternsForFM);
	for (auto pat : patternsForFM) {
		delete pat;
	}
	in->mWords.swap(words);
}

int PartialMatchPattern::Match(
	LPCTSTR str
)
{
	return Match(str, 0);
}

int PartialMatchPattern::Match(
	LPCTSTR str,
	int offset
)
{
	// 空文字の場合はマッチさせない
	if (str == nullptr || str[0] == _T('\0')) {
		return Mismatch;
	}

	// 入力されたキーワードに完全一致するかどうかの判断
	CString keyword;
	if (in->GetKeyword(offset, keyword) && keyword.CompareNoCase(str) == 0) {
		return WholeMatch;
	}

	std::string str_;
	UTF2UTF(std::wstring(str), str_);
	if (str_ == "cmd") {
		int a = 0;
	}

	size_t regPatCount = in->mRegPatterns.size();
	for (size_t i = offset; i < regPatCount; ++i) {

		// ひとつでもマッチしないものがあったら、ヒットしないものとみなす
		auto& pattern = *in->mRegPatterns[i];
		if (RE2::PartialMatch(str_, pattern) == false) {
			return Mismatch;
		}
	}

	// 先頭のキーワードに前方一致する場合は前方一致とみなす
	if (offset < in->mRegPatternsForFM.size()) {
		auto& patForFM = *in->mRegPatternsForFM[offset];
		if (RE2::PartialMatch(str_, patForFM)) {
			return FrontMatch;
		}
	}

	// そうでなければ部分一致
	return PartialMatch;
}

LPCTSTR PartialMatchPattern::GetFirstWord()
{
	return in->mTokens.empty() ? _T("") : in->mTokens[0];
}

LPCTSTR PartialMatchPattern::GetWholeString()
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


CString PartialMatchPattern::StripEscapeChars(const CString& pattern)
{
	// 後段のwregexに値を渡したときにエスケープ記号として解釈されるのを防ぐため

	CString tmp(pattern);
	tmp.Replace(_T("\\"), _T("\\\\"));
	tmp.Replace(_T("?"), _T("\\?"));
	tmp.Replace(_T("."), _T("\\."));
	tmp.Replace(_T("*"), _T("\\*"));
	tmp.Replace(_T("+"), _T("\\+"));
	tmp.Replace(_T("^"), _T("\\^"));
	tmp.Replace(_T("$"), _T("\\$"));
	tmp.Replace(_T("["), _T("\\["));
	tmp.Replace(_T("]"), _T("\\]"));
	tmp.Replace(_T("|"), _T("\\|"));
	tmp.Replace(_T("("), _T("\\("));
	tmp.Replace(_T(")"), _T("\\)"));
	tmp.Replace(_T("{"), _T("\\{"));
	tmp.Replace(_T("}"), _T("\\}"));
	tmp.Replace(_T("!"), _T("\\!"));
	tmp.Replace(_T("&"), _T("\\&"));

	return tmp;
}

