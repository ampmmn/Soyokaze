#include "pch.h"
#include "ClipboardMacro.h"
#include "SharedHwnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace macros {
namespace builtin {

REGISTER_LAUNCHERMACRO(ClipboardMacro)

ClipboardMacro::ClipboardMacro()
{
	mName = _T("env");
}

ClipboardMacro::~ClipboardMacro()
{
}

bool ClipboardMacro::Evaluate(const std::vector<CString>& args, CString& result)
{
	SharedHwnd sharedHwnd;
	HWND hwnd = sharedHwnd.GetHwnd();

	if (OpenClipboard(hwnd) == FALSE) {
		return false;
	}

	if (IsClipboardFormatAvailable(CF_UNICODETEXT) == FALSE) {
		CloseClipboard();
		return false;
	}

	HANDLE h = GetClipboardData(CF_UNICODETEXT);
	LPCTSTR p = (LPCTSTR)GlobalLock(h);

	result = p;

	GlobalUnlock(h);
	CloseClipboard();

	return true;
}


}
}
}
