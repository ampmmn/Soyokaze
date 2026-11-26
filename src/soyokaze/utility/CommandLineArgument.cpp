#include "pch.h"
#include "CommandLineArgument.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace utility {

namespace {

// 値の中のダブルクォートとバックスラッシュを、MSVCの解析規則(CommandLineToArgvW)と
// cmd.exeの解析規則の両方で安全な形にエスケープする
//
// - 埋め込みのダブルクォートは""にする
//   (子プロセス側のCommandLineToArgvWではリテラルの1文字として解釈される)
//   (cmd.exe側では引用符のON/OFFが保たれたまま変化しないため、続く&や|が
//    コマンド区切りとして解釈されない)
// - ダブルクォートに続くバックスラッシュ、および末尾のバックスラッシュは2倍にする
//   (MSVCの規則では、引用符に続く奇数個のバックスラッシュが引用符をエスケープするため)
CString EscapeValue(const CString& value)
{
	CString result;

	int length = value.GetLength();
	int index = 0;
	while (index < length) {
		TCHAR c = value[index];

		if (c == _T('\\')) {
			// バックスラッシュの連続を数える
			int count = 0;
			while (index < length && value[index] == _T('\\')) {
				++count;
				++index;
			}
			// 直後がダブルクォート、または値が終わる場合に2倍にする
			bool needsEscape = (index >= length) || (value[index] == _T('"'));
			int emitCount = needsEscape ? count * 2 : count;
			for (int i = 0; i < emitCount; ++i) {
				result += _T('\\');
			}
			continue;
		}

		if (c == _T('"')) {
			// 埋め込みのダブルクォートは""にする
			result += _T("\"\"");
			++index;
			continue;
		}

		result += c;
		++index;
	}

	return result;
}

// 名前を指定して環境変数が定義済みかどうかを判定する
bool IsDefinedEnvVariable(const CString& name)
{
	if (name.IsEmpty()) {
		return false;
	}

	DWORD size = GetEnvironmentVariableW(name, nullptr, 0);
	return size != 0;
}

// 引用符が直前のバックスラッシュでエスケープされているか
bool IsEscapedQuote(const CString& text, int index)
{
	int backslashCount = 0;
	for (int pos = index - 1; pos >= 0 && text[pos] == _T('\\'); --pos) {
		++backslashCount;
	}
	return (backslashCount % 2) != 0;
}

// コマンドライン上の1引数を、Windowsの規則で解析する
bool ParseArgument(const CString& text, CString& value)
{
	CString commandLine = _T("dummy ") + text;
	int argc = 0;
	LPWSTR* argv = ::CommandLineToArgvW(commandLine, &argc);
	if (argv == nullptr) {
		return false;
	}

	bool isParsed = argc == 2;
	if (isParsed) {
		value = argv[1];
	}
	::LocalFree(argv);
	return isParsed;
}

// プレースホルダを含む引数を置換し、引数全体を引用する
CString ReplacePlaceholderInArgument(const CString& argument, LPCTSTR placeholder, const CString& value)
{
	if (argument.Find(placeholder) < 0) {
		return argument;
	}

	CString parsedArgument;
	if (ParseArgument(argument, parsedArgument) == false) {
		// 不正な引数テンプレートでも、選択値を無引用で挿入しない
		CString escapedArgument = argument;
		CString quotedValue = QuoteArgument(value);
		int pos = 0;
		int placeholderLength = static_cast<int>(_tcslen(placeholder));
		while ((pos = escapedArgument.Find(placeholder, pos)) >= 0) {
			escapedArgument.Delete(pos, placeholderLength);
			escapedArgument.Insert(pos, quotedValue);
			pos += quotedValue.GetLength();
		}
		return escapedArgument;
	}

	parsedArgument.Replace(placeholder, value);
	return QuoteArgument(parsedArgument);
}

} // namespace

/**
  コマンドライン引数をMSVCの規則(CommandLineToArgvW)に従ってクォートする
  @param[in] value クォート対象の値
  @return クォート後の文字列
*/
CString QuoteArgument(const CString& value)
{
	return _T("\"") + EscapeValue(value) + _T("\"");
}

/**
  QuoteArgumentの、外側のダブルクォートを付けない版
  @param[in] value エスケープ対象の値
  @return エスケープ後の文字列
*/
CString EscapeArgumentValue(const CString& value)
{
	return EscapeValue(value);
}

/**
  値にcmd.exeによって展開される環境変数参照(%NAME%)が含まれるか
  @param[in] value 判定対象の値
  @return 展開される環境変数参照が含まれる場合true
*/
bool ContainsExpandableEnvReference(const CString& value)
{
	int length = value.GetLength();
	int index = 0;
	while (index < length) {
		if (value[index] != _T('%')) {
			++index;
			continue;
		}

		// 閉じの%を探す(環境変数名に相当する範囲のみを調べる)
		int nameEnd = index + 1;
		while (nameEnd < length && value[nameEnd] != _T('%')) {
			TCHAR c = value[nameEnd];
			if (c == _T('=') || c == _T(' ') || c == _T('\t') ||
				c == _T('\r') || c == _T('\n') || c == _T('"')) {
				break;
			}
			++nameEnd;
		}

		if (nameEnd < length && value[nameEnd] == _T('%') && nameEnd > index + 1) {
			// %NAME%形式。定義済みの環境変数を参照していればcmd.exeによって展開される
			CString name(value.Mid(index + 1, nameEnd - index - 1));
			if (IsDefinedEnvVariable(name)) {
				return true;
			}
			// 判定した閉じの%の次から続行する
			index = nameEnd + 1;
			continue;
		}

		// 閉じの%が見つからない場合は1文字だけ進む
		++index;
	}

	return false;
}

/**
  コマンドライン中のプレースホルダを、引数単位で安全に置換する
  プレースホルダを含む引数全体を引用し、値が引数の一部でも正しく渡す
  @param[in,out] text 置換する文字列
  @param[in] placeholder 置換対象のプレースホルダ
  @param[in] value 置き換える値
*/
void ReplaceQuotedPlaceholder(CString& text, LPCTSTR placeholder, const CString& value)
{
	CString result;
	int length = text.GetLength();
	int tokenStart = 0;
	bool isInsideQuotes = false;

	for (int index = 0; index <= length; ++index) {
		bool isEnd = index == length;
		bool isSeparator = isEnd ||
			((text[index] == _T(' ') || text[index] == _T('\t')) && isInsideQuotes == false);
		if (isSeparator) {
			CString argument = text.Mid(tokenStart, index - tokenStart);
			result += ReplacePlaceholderInArgument(argument, placeholder, value);

			if (isEnd == false) {
				result += text[index];
				tokenStart = index + 1;
			}
			continue;
		}

		if (text[index] == _T('"') && IsEscapedQuote(text, index) == false) {
			isInsideQuotes = !isInsideQuotes;
		}
	}

	text = result;
}

}
}
