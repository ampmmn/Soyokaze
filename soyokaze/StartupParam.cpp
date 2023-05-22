#include "pch.h"
#include "StartupParam.h"
#include "Arguments.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct StartupParam::PImpl
{
public:
	PImpl(int argc, TCHAR* argv[]) : mArgs(argc, argv)
	{
	}

	Arguments mArgs;
};


StartupParam::StartupParam(int argc, TCHAR* argv[]) : 
	in(new PImpl(argc, argv))
{
}

StartupParam::~StartupParam()
{
}

bool StartupParam::HasRunCommand(CString& commands)
{
	return in->mArgs.GetBWOptValue(_T("/Runcommand="), commands) ||
	       in->mArgs.GetValue(_T("-c"), commands);
}


bool StartupParam::HasPathToRegister(CString& pathToRegister)
{
	if (in->mArgs.GetCount() > 1 && PathFileExists(in->mArgs.Get(1))) {
		pathToRegister = in->mArgs.Get(1);
		return true;
	}
	return false;
}
