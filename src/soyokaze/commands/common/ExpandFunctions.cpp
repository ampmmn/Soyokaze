#include "pch.h"
#include "ExpandFunctions.h"
#include "macros/core/MacroRepository.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace common {

static bool HasSpace(const CString& arg)
{
	return arg.Find(_T(' ')) != -1;
}

void ExpandArguments(
	CString& target,
	const std::vector<CString>& args
)
{
	CString argAll;

	// $1...$nを置換する
	TCHAR key[32];
	for (int i = 0; i < (int)args.size(); ++i) {

		CString arg;

		bool hasSpace = HasSpace(args[i]);
		if (hasSpace) {
			arg = _T("\"");
			arg += args[i];
			arg += _T("\"");
		}
		else {
			arg = args[i];
		}

		_stprintf_s(key, _T("$%d"), i+1);

		target.Replace(key, arg);

		// あわせて、$*用の置換後の文字列を生成する
		if (i != 0) {
			argAll += _T(" ");
		}

		argAll += arg;
	}


	// $*を置換する
	if (target.Find(_T("$*")) != -1) {
		target.Replace(_T("$*"), argAll);
	}
}

// ランチャーのマクロ機能による置換
bool ExpandMacros(CString& target)
{
	auto macroRepos = launcherapp::macros::core::MacroRepository::GetInstance();
	return macroRepos->Evaluate(target);
}

// 前後のダブルクォーテーションを除去する
void StripDoubleQuate(CString& str)
{
	if (str.GetLength() >= 2 && str[0] == _T('"') && str[str.GetLength()-1] == _T('"')) {
		str = str.Mid(1, str.GetLength()-2);
	}
}


} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp


