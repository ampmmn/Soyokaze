#include "pch.h"
#include "ExpandFunctions.h"

namespace soyokaze {
namespace commands {
namespace common {


void ExpandArguments(
	CString& target,
	const std::vector<CString>& args
)
{
	CString argAll;

	// $1...$nを置換する
	TCHAR key[32];
	for (int i = 0; i < (int)args.size(); ++i) {

		_stprintf_s(key, _T("$%d"), i+1);

		target.Replace(key, args[i]);

		// あわせて、$*用の置換後の文字列を生成する
		if (i != 0) {
			argAll += _T(" ");
		}
		argAll += args[i];
	}


	// $*を置換する
	if (target.Find(_T("$*")) != -1) {
		target.Replace(_T("$*"), argAll);
	}
}

void ExpandEnv(CString& target)
{
	CString workBuf(target);

	int len = target.GetLength();
	for (int i = 0; i < len; ++i) {
		TCHAR c = target[i];

		if (c != _T('$')) {
			continue;
		}
		if (i + 1 >= len) {
			break;
		}

		int startPos = i + 1;

		for (int j = startPos; j < len; ++j) {

			c = target[j];

			bool isa = (_T('a') <= c && c <= _T('z'));
			bool isA = (_T('A') <= c && c <= _T('Z'));
			bool isnum = (_T('0') <= c && c <= _T('9'));
			bool is_ = (c == _T('_') || c == _T('(') || c == _T(')'));

			if (isa || isA || isnum || is_) {
				continue;
			}

			CString valName = target.Mid(startPos, j - startPos);
			if (valName.IsEmpty()) {
				i = j - 1;
				startPos = -1;
				break;
			}

			size_t reqLen = 0;
			if (_tgetenv_s(&reqLen, NULL, 0, valName) != 0 || reqLen == 0) {
				i = j - 1;
				startPos = -1;
				break;
			}

			CString val;
			TCHAR* p = val.GetBuffer((int)reqLen);
			_tgetenv_s(&reqLen, p, reqLen, valName);
			val.ReleaseBuffer();

			CString before(_T("$"));
			before += valName;

			workBuf.Replace(before, val);
			startPos = -1;

			i = j - 1;
			break;
		}
		if (startPos == -1) {
			continue;
		}

		CString valName = target.Mid(startPos, len - startPos);
		if (valName.IsEmpty()) {
			break;
		}

		size_t reqLen = 0;
		if (_tgetenv_s(&reqLen, NULL, 0, valName) != 0 || reqLen == 0) {
			break;
		}

		CString val;
		TCHAR* p = val.GetBuffer((int)reqLen);
		_tgetenv_s(&reqLen, p, reqLen, valName);
		val.ReleaseBuffer();

		CString before(_T("$"));
		before += valName;

		workBuf.Replace(before, val);
		break;
	}

	target = workBuf;
}

void ExpandVariable(
	CString& target,
 	const CString& name,
 	const CString& value
)
{
	CString pat(_T("$"));
	pat += name;

	// ${name}を置換する
	if (target.Find(pat) != -1) {
		target.Replace(pat, value);
	}
}

bool ResolaveRelativeExePath(CString& text)
{
	if (PathIsRelative(text) == FALSE) {
		// 相対パスでないなら処理しない
		return true;
	}

	// 拡張子が.exeでなければ末尾に付与
	CString ext(_T(".exe"));
	if (ext != PathFindExtension(text)) {
		text += _T(".exe");
	}
	

	// 環境変数PATH
	LPCTSTR PATH = _T("PATH");
	size_t reqLen = 0;
	if (_tgetenv_s(&reqLen, NULL, 0, PATH) != 0 || reqLen == 0) {
		return false;
	}

	// 個々の要素に分ける
	CString val;
	TCHAR* p = val.GetBuffer((int)reqLen);
	_tgetenv_s(&reqLen, p, reqLen, PATH);
	val.ReleaseBuffer();


	std::vector<CString> targetDirs;

	int n = 0;
	CString item = val.Tokenize(_T(";"), n);
	while(item.IsEmpty() == FALSE) {

		if (PathIsDirectory(item)) {
			targetDirs.push_back(item);
		}

		item = val.Tokenize(_T(";"), n);
	}

	// 個々の分けた要素が示すパス下に同名のexeがあるかを探す
	TCHAR path[MAX_PATH_NTFS];
	for (const auto& dir : targetDirs) {
		_tcscpy_s(path, dir);
		PathAppend(path, text);

		if (PathFileExists(path) == FALSE) {
			continue;
		}
		text = path;
		return true;
	}

	// ない
	return false;
}


} // end of namespace common
} // end of namespace commands
} // end of namespace soyokaze


