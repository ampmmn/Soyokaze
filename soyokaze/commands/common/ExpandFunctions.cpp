#include "pch.h"
#include "ExpandFunctions.h"
#include "SharedHwnd.h"
#include "AfxWWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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

void ExpandClipboard(CString& target)
{
	SharedHwnd sharedHwnd;
	HWND hwnd = sharedHwnd.GetHwnd();

	if (OpenClipboard(hwnd) == FALSE) {
		return ;
	}

	if (IsClipboardFormatAvailable(CF_UNICODETEXT) == FALSE) {
		CloseClipboard();
		return ;
	}

	HANDLE h = GetClipboardData(CF_UNICODETEXT);
	LPCTSTR p = (LPCTSTR)GlobalLock(h);

	target.Replace(_T("$clipboard"), p);

	GlobalUnlock(h);
	CloseClipboard();
}

// あふwで現在表示しているディレクトリを表示する
bool ExpandAfxCurrentDir(CString& target)
{
	if (target.Find(_T("$afxcurrentdir")) == -1) {
		// キーワード無し
		return true;
	}

	AfxWWrapper afxw;
	CString curDir = afxw.GetCurrentDir();
	target.Replace(_T("$afxcurrentdir"), curDir);

	return true;
}


} // end of namespace common
} // end of namespace commands
} // end of namespace soyokaze


