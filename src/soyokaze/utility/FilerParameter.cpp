#include "pch.h"
#include "FilerParameter.h"

namespace launcherapp { namespace utility {

/**
  ファイラー起動時の引数に対象パスを展開する
  @param[in,out] parameter ファイラーに渡す引数
  @param[in] targetPath 展開する対象パス
*/
void ExpandFilerParameter(CString& parameter, const CString& targetPath)
{
	// スラッシュ形式の指定では、対象パスの区切り文字だけを変換する
	CString slashPath(targetPath);
	slashPath.Replace(_T('\\'), _T('/'));
	bool shouldQuote = targetPath.FindOneOf(_T(" \t\r\n\v\f")) >= 0;

	const CString slashToken(_T("${target:s}"));
	const CString targetToken(_T("$target"));
	CString expanded;
	int position = 0;
	while (position < parameter.GetLength()) {
		const CString* replacement = nullptr;
		int tokenLength = 0;
		if (parameter.Mid(position, slashToken.GetLength()) == slashToken) {
			replacement = &slashPath;
			tokenLength = slashToken.GetLength();
		}
		else if (parameter.Mid(position, targetToken.GetLength()) == targetToken) {
			replacement = &targetPath;
			tokenLength = targetToken.GetLength();
		}
		else {
			expanded += parameter[position];
			++position;
			continue;
		}

		bool isQuoted = position > 0 && position + tokenLength < parameter.GetLength() &&
			parameter[position - 1] == _T('"') && parameter[position + tokenLength] == _T('"');
		if (shouldQuote && isQuoted == false) {
			expanded += _T('"');
		}
		expanded += *replacement;
		if (shouldQuote && isQuoted == false) {
			expanded += _T('"');
		}
		position += tokenLength;
	}
	parameter = expanded;
}

}}
