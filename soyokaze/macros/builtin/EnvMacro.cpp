#include "pch.h"
#include "EnvMacro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace macros {
namespace builtin {

REGISTER_LAUNCHERMACRO(EnvMacro)

EnvMacro::EnvMacro()
{
	mName = _T("env");
}

EnvMacro::~EnvMacro()
{
}

bool EnvMacro::Evaluate(const std::vector<CString>& args, CString& result)
{
	if (args.size() == 0) {
		return false;
	}

	CString valName = args[0];

	size_t reqLen = 0;
	if (_tgetenv_s(&reqLen, NULL, 0, valName) != 0 || reqLen == 0) {
		// 指定された名前の変数はない
		return false;
	}

	TCHAR* p = result.GetBuffer((int)reqLen);
	_tgetenv_s(&reqLen, p, reqLen, valName);
	result.ReleaseBuffer();

	return true;
}


}
}
}
