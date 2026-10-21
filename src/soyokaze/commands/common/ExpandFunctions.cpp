#include "pch.h"
#include "ExpandFunctions.h"
#include "macros/core/MacroRepository.h"
#include "utility/CommandLineArgument.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace common {

namespace {

// 展開用プレースホルダの開始文字
// (マクロの解析対象にならない制御文字を使う)
constexpr TCHAR PLACEHOLDER_MARK = _T('\x01');

// 展開対象として保持する情報
struct Replacement {
	CString value;         // 置き換える値
	bool isAll{false};     // $* (すべての実行時引数)かどうか
	bool isQuoted{false};  // テンプレート側でダブルクォートが付いているかどうか
};

// 展開用プレースホルダを生成する
CString MakePlaceholder(int index)
{
	CString placeholder;
	placeholder.Format(_T("\x01%d\x01"), index);
	return placeholder;
}

// プレースホルダがテンプレート側のダブルクォートで囲まれているか
bool IsSurroundedByQuote(const CString& target, int start, int end)
{
	if (start <= 0 || end >= target.GetLength()) {
		return false;
	}
	return target[start - 1] == _T('"') && target[end] == _T('"');
}

// すべての実行時引数を連結して返す
CString JoinArguments(const std::vector<CString>& args, ArgumentQuoting quoting, bool isQuoted)
{
	CString result;

	for (size_t i = 0; i < args.size(); ++i) {
		if (i != 0) {
			result += _T(" ");
		}

		if (quoting == ArgumentQuoting::None) {
			result += args[i];
		}
		else if (isQuoted) {
			// テンプレート側で引用符が付いている場合は内側だけエスケープする
			result += utility::EscapeArgumentValue(args[i]);
		}
		else {
			result += utility::QuoteArgument(args[i]);
		}
	}

	return result;
}

// 展開対象の値をクォート/エスケープして返す
CString FormatValue(const Replacement& rep, ArgumentQuoting quoting, const std::vector<CString>& args)
{
	if (rep.isAll) {
		return JoinArguments(args, quoting, rep.isQuoted);
	}

	if (quoting == ArgumentQuoting::None) {
		return rep.value;
	}
	if (rep.isQuoted) {
		// テンプレート側で引用符が付いている場合は内側だけエスケープする
		return utility::EscapeArgumentValue(rep.value);
	}
	return utility::QuoteArgument(rep.value);
}

// テンプレートを走査して、実行時引数のプレースホルダへ置換した文字列を作成する
void ScanTemplate(const CString& target, const std::vector<CString>& args, CString& scanned, std::vector<Replacement>& replacements)
{
	scanned = _T("");
	replacements.clear();

	int length = target.GetLength();
	int index = 0;
	while (index < length) {
		if (target[index] == _T('$') && index + 1 < length) {
			TCHAR next = target[index + 1];

			if (next == _T('*')) {
				// $* (すべての実行時引数)
				Replacement rep;
				rep.isAll = true;
				rep.isQuoted = IsSurroundedByQuote(target, index, index + 2);

				replacements.push_back(rep);
				scanned += MakePlaceholder((int)replacements.size() - 1);
				index += 2;
				continue;
			}

			if (next >= _T('0') && next <= _T('9')) {
				// $1...$n (連続する数字は1つの引数番号として扱う)
				int end = index + 1;
				int number = 0;
				bool overflow = false;
				while (end < length && target[end] >= _T('0') && target[end] <= _T('9')) {
					if (number > 100000) {
						overflow = true;
						break;
					}
					number = number * 10 + (target[end] - _T('0'));
					++end;
				}

				if (overflow == false && number >= 1 && number <= (int)args.size()) {
					Replacement rep;
					rep.value = args[number - 1];
					rep.isQuoted = IsSurroundedByQuote(target, index, end);

					replacements.push_back(rep);
					scanned += MakePlaceholder((int)replacements.size() - 1);
					index = end;
					continue;
				}

				// 範囲外の引数番号の場合、1桁少ない記載があればそちらを優先する
				// (例: 引数が2つのときの$12は$1と"2"として扱う)
				if (overflow == false && end - index >= 3) {
					int prefix = number / 10;
					if (prefix >= 1 && prefix <= (int)args.size()) {
						Replacement rep;
						rep.value = args[prefix - 1];
						rep.isQuoted = IsSurroundedByQuote(target, index, end - 1);

						replacements.push_back(rep);
						scanned += MakePlaceholder((int)replacements.size() - 1);
						index = end - 1;
						continue;
					}
				}

				// 引数番号として解釈できない場合はそのまま残す
				scanned += _T('$');
				++index;
				continue;
			}
		}

		scanned += target[index];
		++index;
	}
}

// プレースホルダを実行時引数の値へ置き換える
// (置き換え後の値は再度走査しない)
void ResolvePlaceholders(CString& text, const std::vector<Replacement>& replacements, ArgumentQuoting quoting, const std::vector<CString>& args)
{
	CString result;

	int length = text.GetLength();
	int index = 0;
	while (index < length) {
		if (text[index] == PLACEHOLDER_MARK) {
			int end = index + 1;
			int number = 0;
			int digits = 0;
			while (end < length && text[end] >= _T('0') && text[end] <= _T('9')) {
				number = number * 10 + (text[end] - _T('0'));
				++digits;
				++end;
			}

			if (digits > 0 && end < length && text[end] == PLACEHOLDER_MARK &&
				number >= 0 && number < (int)replacements.size()) {
				result += FormatValue(replacements[number], quoting, args);
				index = end + 1;
				continue;
			}
		}

		result += text[index];
		++index;
	}

	text = result;
}

} // namespace

/**
  実行時引数とマクロをまとめて展開する
  展開された値が再度展開されることはない
  @param[in,out] target 展開する文字列
  @param[in] args 実行時引数
  @param[in] quoting 値のクォート方法
  @param[in] expandMacros マクロを展開するかどうか
*/
void ExpandArgumentsAndMacros(CString& target, const std::vector<CString>& args, ArgumentQuoting quoting, bool expandMacros)
{
	// 実行時引数をプレースホルダへ置換する
	CString scanned;
	std::vector<Replacement> replacements;
	ScanTemplate(target, args, scanned, replacements);

	// マクロを展開する
	// (この時点では実行時引数はプレースホルダのままなので、
	//  マクロの展開結果に含まれる$nなどが解釈されることはない)
	if (expandMacros) {
		auto macroRepos = launcherapp::macros::core::MacroRepository::GetInstance();
		macroRepos->Evaluate(scanned, quoting == ArgumentQuoting::Always);
	}

	// プレースホルダを実行時引数の値へ戻す
	// (値は最後に挿入されるため、再度解釈されることはない)
	ResolvePlaceholders(scanned, replacements, quoting, args);

	target = scanned;
}

/**
  実行時引数とマクロをまとめて展開する
  展開された値が再度展開されることはない
  @param[in,out] target 展開する文字列
  @param[in] param 実行時引数
  @param[in] quoting 値のクォート方法
  @param[in] expandMacros マクロを展開するかどうか
*/
void ExpandArgumentsAndMacros(CString& target, const launcherapp::actions::core::Parameter* param, ArgumentQuoting quoting, bool expandMacros)
{
	std::vector<CString> args;

	if (param) {
		int paramCount = param->GetParamCount();
		args.reserve(paramCount);
		for (int i = 0; i < paramCount; ++i) {
			args.push_back(param->GetParam(i));
		}
	}

	ExpandArgumentsAndMacros(target, args, quoting, expandMacros);
}

/**
  実行時引数($1...$n, $*)を展開する
  @param[in,out] target 展開する文字列
  @param[in] args 実行時引数
  @param[in] quoting 値のクォート方法
*/
void ExpandArguments(CString& target, const std::vector<CString>& args, ArgumentQuoting quoting)
{
	ExpandArgumentsAndMacros(target, args, quoting, false);
}

/**
  実行時引数($1...$n, $*)を展開する
  @param[in,out] target 展開する文字列
  @param[in] param 実行時引数
  @param[in] quoting 値のクォート方法
*/
void ExpandArguments(CString& target, const launcherapp::actions::core::Parameter* param, ArgumentQuoting quoting)
{
	std::vector<CString> args;

	if (param) {
		int paramCount = param->GetParamCount();
		args.reserve(paramCount);
		for (int i = 0; i < paramCount; ++i) {
			args.push_back(param->GetParam(i));
		}
	}

	ExpandArgumentsAndMacros(target, args, quoting, false);
}

/**
  ランチャーのマクロ機能による置換
  @param[in,out] target 置換する文字列
  @return 置換に成功した場合true
*/
bool ExpandMacros(CString& target)
{
	auto macroRepos = launcherapp::macros::core::MacroRepository::GetInstance();
	return macroRepos->Evaluate(target);
}

/**
  前後のダブルクォーテーションを除去する
  @param[in,out] str 対象の文字列
*/
void StripDoubleQuate(CString& str)
{
	if (str.GetLength() >= 2 && str[0] == _T('"') && str[str.GetLength()-1] == _T('"')) {
		str = str.Mid(1, str.GetLength()-2);
	}
}

} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp
