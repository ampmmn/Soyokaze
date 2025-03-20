#include "pch.h"
#include "PathWatcherItem.h"

namespace launcherapp {
namespace commands {
namespace watchpath {


/**
	除外パターンから正規表現オブジェクトを作成する
 	@return true:成功  false:失敗
 	@param[out] reg 生成された正規表現オブジェクト
*/
bool PathWatcherItem::BuildExcludeFilterRegex(
	std::unique_ptr<tregex>& reg
) const
{
	tstring wholePattern;

	int n = 0;
	CString tok = mExcludeFilter.Tokenize(_T(","), n);
	while(tok.IsEmpty() == FALSE) {

		tok.Trim();

		tstring patStr;
		WildcardToRegexp(tok, patStr);

		if (wholePattern.empty() == false) {
			wholePattern += _T("|");
		}
		wholePattern += _T("(?:");
		wholePattern += patStr;
		wholePattern += _T(")");

		tok = mExcludeFilter.Tokenize(_T(","), n);
	}

	try {
		std::unique_ptr<tregex> tmpReg(new tregex(wholePattern, std::regex_constants::icase));
		tmpReg.swap(reg);
		return true;
	}
	catch(...) {
		spdlog::warn(_T("Failed to build regex {}"), (LPCTSTR)mExcludeFilter);
		return false;
	}
}

void PathWatcherItem::WildcardToRegexp(const CString& in, tstring& out)
{
    tstring tmp;
    for (int i = 0; i < in.GetLength(); ++i) {

        auto c = in[i];

        if (c == '*') { tmp += _T(".*"); continue; }
        if (c == '?') { tmp += _T("."); continue; }

        if (c == _T('.') || c == _T('(') || c == _T(')') || c == _T('[') || c == _T(']') ||
            c == _T('{') || c == _T('}') || c == _T('|') || c == _T('^') || c == _T('$') ||
            c == _T('\\') || c == _T('+')) {
            tmp += _T('\\');
            tmp += c;
            continue;
        }
        tmp += c;
    }
    out.swap(tmp);
}



}
}
}


