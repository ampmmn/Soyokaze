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

	const CString slashToken(_T("${target:s}"));
	const CString targetToken(_T("$target"));
	CString expanded;
	int position = 0;
	while (position < parameter.GetLength()) {
		if (parameter.Mid(position, slashToken.GetLength()) == slashToken) {
			expanded += slashPath;
			position += slashToken.GetLength();
		}
		else if (parameter.Mid(position, targetToken.GetLength()) == targetToken) {
			expanded += targetPath;
			position += targetToken.GetLength();
		}
		else {
			expanded += parameter[position];
			++position;
		}
	}
	parameter = expanded;
}

}}
