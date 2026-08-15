#pragma once

#include <memory>
#include <string>
#include <vector>
#include <re2/re2.h>

namespace launcherapp {
namespace utility {

/**
  RE2を使用した正規表現マッチングを行う
*/
class Regex
{
public:
	Regex() = default;
	Regex(const CString& pattern, bool caseSensitive = true)
	{
		Compile(pattern, caseSensitive);
	}
	Regex(const Regex& rhs)
	{
		if (rhs.mPattern.IsEmpty() == false) {
			Compile(rhs.mPattern, rhs.mCaseSensitive);
		}
	}
	Regex& operator=(const Regex& rhs)
	{
		if (this != &rhs) {
			mRegex.reset();
			mPattern.Empty();
			mError.clear();
			mCaseSensitive = rhs.mCaseSensitive;
			if (rhs.mPattern.IsEmpty() == false) {
				Compile(rhs.mPattern, rhs.mCaseSensitive);
			}
		}
		return *this;
	}

	/**
 	正規表現をコンパイルする
 	@return true:成功 false:失敗
 	@param[in] pattern 正規表現パターン
 	@param[in] caseSensitive 大文字・小文字を区別するかどうか
 */
	bool Compile(const CString& pattern, bool caseSensitive = true)
	{
		mPattern = pattern;
		mCaseSensitive = caseSensitive;
		mError.clear();

		std::string patternUtf8;
		UTF2UTF(std::wstring((LPCWSTR)pattern), patternUtf8);

		RE2::Options options;
		options.set_case_sensitive(caseSensitive);
		std::unique_ptr<RE2> regex(new RE2(patternUtf8, options));
		if (regex->ok() == false) {
			mError = regex->error();
			mRegex.reset();
			return false;
		}
		mRegex.swap(regex);
		return true;
	}

	bool IsValid() const
	{
		return mRegex != nullptr && mRegex->ok();
	}

	const std::string& GetError() const
	{
		return mError;
	}

	/**
 	文字列全体が正規表現に一致するか確認する
 	@return true:一致 false:不一致
 	*/
	bool FullMatch(const CString& text) const
	{
		return Match(text, RE2::ANCHOR_BOTH, nullptr);
	}

	bool FullMatch(const std::wstring& text) const
	{
		return Match(text, RE2::ANCHOR_BOTH, nullptr);
	}

	/**
 	文字列リテラル全体が正規表現に一致するか確認する
 	@return true:一致 false:不一致
 	*/
	bool FullMatch(const wchar_t* text) const
	{
		return FullMatch(std::wstring(text));
	}

	/**
 	文字列の一部が正規表現に一致するか確認する
 	@return true:一致 false:不一致
 	*/
	bool PartialMatch(const CString& text) const
	{
		return Match(text, RE2::UNANCHORED, nullptr);
	}

	bool PartialMatch(const std::wstring& text) const
	{
		return Match(text, RE2::UNANCHORED, nullptr);
	}

	/**
 	文字列リテラルの一部が正規表現に一致するか確認する
 	@return true:一致 false:不一致
 	*/
	bool PartialMatch(const wchar_t* text) const
	{
		return PartialMatch(std::wstring(text));
	}

	/**
 	正規表現に一致したキャプチャ文字列を取得する
 	@return true:一致 false:不一致
 	*/
	bool FullMatch(const CString& text, std::vector<CString>& captures) const
	{
		return MatchWithCaptures(text, RE2::ANCHOR_BOTH, captures);
	}

	/**
 	正規表現に部分一致したキャプチャ文字列を取得する
 	@return true:一致 false:不一致
 	*/
	bool PartialMatch(const CString& text, std::vector<CString>& captures) const
	{
		return MatchWithCaptures(text, RE2::UNANCHORED, captures);
	}

	/**
 	一致した部分をすべて置換する
 	@return true:置換処理を実行できた false:正規表現が無効
 	*/
	bool GlobalReplace(const CString& text, const CString& rewrite, CString& result) const
	{
		if (IsValid() == false) {
			return false;
		}
		std::string textUtf8;
		std::string rewriteUtf8;
		UTF2UTF(std::wstring((LPCWSTR)text), textUtf8);
		UTF2UTF(std::wstring((LPCWSTR)rewrite), rewriteUtf8);
		RE2::GlobalReplace(&textUtf8, *mRegex, rewriteUtf8);
		UTF2UTF(textUtf8, result);
		return true;
	}

private:
	bool Match(const CString& text, RE2::Anchor anchor, std::vector<CString>* captures) const
	{
		UNREFERENCED_PARAMETER(captures);
		if (IsValid() == false) {
			return false;
		}

		std::string textUtf8;
		UTF2UTF(std::wstring((LPCWSTR)text), textUtf8);
		return mRegex->Match(textUtf8, 0, textUtf8.size(), anchor, nullptr, 0);
	}

	bool Match(const std::wstring& text, RE2::Anchor anchor, std::vector<CString>* captures) const
	{
		UNREFERENCED_PARAMETER(captures);
		if (IsValid() == false) {
			return false;
		}

		std::string textUtf8;
		UTF2UTF(text, textUtf8);
		return mRegex->Match(textUtf8, 0, textUtf8.size(), anchor, nullptr, 0);
	}

	bool MatchWithCaptures(const CString& text, RE2::Anchor anchor, std::vector<CString>& captures) const
	{
		captures.clear();
		if (IsValid() == false) {
			return false;
		}

		std::string textUtf8;
		UTF2UTF(std::wstring((LPCWSTR)text), textUtf8);
		int count = mRegex->NumberOfCapturingGroups() + 1;
		std::vector<absl::string_view> submatches(static_cast<size_t>(count));
		if (mRegex->Match(textUtf8, 0, textUtf8.size(), anchor, submatches.data(), count) == false) {
			return false;
		}

		for (int i = 1; i < count; ++i) {
			CString value;
			if (submatches[i].data() != nullptr) {
				UTF2UTF(std::string(submatches[i].data(), submatches[i].size()), value);
			}
			captures.push_back(value);
		}
		return true;
	}

	CString mPattern;
	bool mCaseSensitive{true};
	std::unique_ptr<RE2> mRegex;
	std::string mError;
};

} // namespace utility
} // namespace launcherapp
