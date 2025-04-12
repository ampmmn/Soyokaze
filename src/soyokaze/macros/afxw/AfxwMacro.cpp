#include "pch.h"
#include "AfxwMacro.h"
#include "commands/share/AfxWWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace macros {
namespace builtin {

REGISTER_LAUNCHERMACRO(AfxwMacro)

AfxwMacro::AfxwMacro()
{
	mName = _T("afxw");
}

AfxwMacro::~AfxwMacro()
{
}

bool AfxwMacro::Evaluate(const std::vector<CString>& args, CString& result)
{
	if (args.size() == 0) {
		return false;
	}

	CString command = args[0];

	if (command.CompareNoCase(_T("currentdir")) != 0) {
		return false;
	}

	AfxWWrapper afxw;
	std::wstring curDir;
	if (afxw.GetCurrentDir(curDir) == false) {
		return false;
	}
	result = curDir.c_str();
	return true;
}


}
}
}
