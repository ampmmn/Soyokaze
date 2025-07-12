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
	if (command.CompareNoCase(_T("currentdir")) == 0 || command.CompareNoCase(_T("location_path")) == 0) {
		return ExpandLocationPath(args, result);
	}
	if (command.CompareNoCase(_T("selection_path")) == 0) {
		return ExpandSelectionPath(args, result);
	}
	return false;
}

bool AfxwMacro::ExpandLocationPath(const std::vector<CString>& args, CString& result)
{
	UNREFERENCED_PARAMETER(args);

	AfxWWrapper afxw;
	std::wstring curDir;
	if (afxw.GetCurrentDir(curDir) == false) {
		return false;
	}
	result = curDir.c_str();
	return true;
}

bool AfxwMacro::ExpandSelectionPath(const std::vector<CString>& args, CString& result)
{
	int index = -1;
	if (args.size() > 1) {
		index = std::stoi(tstring((LPCTSTR)args[1]));
	}

	AfxWWrapper afxw;
	std::wstring path;
	if (afxw.GetSelectionPath(path, index) == false) {
		return false;
	}
	result = path.c_str();
	return true;
}



}
}
}
