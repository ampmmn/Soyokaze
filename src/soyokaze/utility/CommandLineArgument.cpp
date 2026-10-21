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
  文字列中のプレースホルダを、値をエスケープしたうえで置換する
  プレースホルダがダブルクォートで囲まれている場合は内側だけエスケープする
  @param[in,out] text 置換する文字列
  @param[in] placeholder 置換対象のプレースホルダ
  @param[in] value 置き換える値
*/
void ReplaceQuotedPlaceholder(CString& text, LPCTSTR placeholder, const CString& value)
{
	int placeholderLength = static_cast<int>(_tcslen(placeholder));

	int pos = 0;
	while ((pos = text.Find(placeholder, pos)) >= 0) {
		bool isQuoted = pos > 0 && pos + placeholderLength < text.GetLength() &&
			text[pos - 1] == _T('"') && text[pos + placeholderLength] == _T('"');

		CString replacement = isQuoted ?
			EscapeArgumentValue(value) :
			QuoteArgument(value);

		text.Delete(pos, placeholderLength);
		text.Insert(pos, replacement);
		pos = pos + replacement.GetLength();
	}
}

}
}
