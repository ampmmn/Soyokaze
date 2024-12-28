#include "pch.h"
#include "BaseNameMacro.h"
#include "SharedHwnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace macros {
namespace builtin {

REGISTER_LAUNCHERMACRO(BaseNameMacro)

BaseNameMacro::BaseNameMacro()
{
	mName = _T("basename");
}

BaseNameMacro::~BaseNameMacro()
{
}

bool BaseNameMacro::Evaluate(const std::vector<CString>& args, CString& result)
{
	if (args.size() == 0) {
		return false;
	}

	CString path = args[0];
	if (path.IsEmpty()) {
		return false;
	}

	bool isQuate = false;
	if (path.GetLength() > 2 && path[0] == _T('"') && path[path.GetLength()-1] == _T('"')) {
		isQuate = true;
		path = path.Mid(1, path.GetLength()-2);
	}

	PathRemoveFileSpec(path.GetBuffer(path.GetLength()));
	path.ReleaseBuffer();
	if (isQuate) {
		path = _T("\"") + path + _T("\"");
	}

	result = path;

	spdlog::debug(_T("result={}"), (LPCTSTR)path);

	return true;
}


}
}
}
