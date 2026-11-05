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

bool AfxwMacro::Evaluate(const std::vector<CString>& args, std::vector<CString>& results)
{
	results.clear();

	if (args.size() == 0) {
		return false;
	}

	CString command = args[0];
	if (command.CompareNoCase(_T("currentdir")) == 0 || command.CompareNoCase(_T("location_path")) == 0) {
		return ExpandLocationPath(args, results);
	}
	if (command.CompareNoCase(_T("selection_path")) == 0) {
		return ExpandSelectionPath(args, results);
	}
	return false;
}

bool AfxwMacro::ExpandLocationPath(const std::vector<CString>& args, std::vector<CString>& results)
{
	UNREFERENCED_PARAMETER(args);

	AfxWWrapper afxw;
	std::wstring curDir;
	if (afxw.GetCurrentDir(curDir) == false) {
		return false;
	}
	results.emplace_back(curDir.c_str());
	return true;
}

bool AfxwMacro::ExpandSelectionPath(const std::vector<CString>& args, std::vector<CString>& results)
{
	int index = -1;
	if (args.size() > 1) {
		index = std::stoi(tstring((LPCTSTR)args[1]));
	}

	AfxWWrapper afxw;
	std::vector<std::wstring> paths;
	if (afxw.GetSelectionPath(paths, index) == false) {
		return false;
	}
	for (auto& path : paths) {
		results.emplace_back(path.c_str());
	}
	return true;
}



}
}
}
