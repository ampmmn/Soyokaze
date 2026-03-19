#include "pch.h"
#include "Win32MenuParam.h"
#include "setting/Settings.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace win32menu {

CommandParam::CommandParam()
{
}


CommandParam::CommandParam(const CommandParam& rhs)
{
	mIsEnable = rhs.mIsEnable;
	mPrefix = rhs.mPrefix;
}

CommandParam::~CommandParam()
{
}

CommandParam& CommandParam::operator = (const CommandParam& rhs)
{
	if (&rhs != this) {
		mIsEnable = rhs.mIsEnable;
		mPrefix = rhs.mPrefix;
	}
	return *this;
}

bool CommandParam::Load(Settings& settings)
{
	mIsEnable = settings.Get(_T("UIAutomationSetting:IsEnableMenuItem"), false);
	mPrefix = settings.Get(_T("UIAutomationSetting:Prefix"), _T("ui"));
	return true;
}

}}} // end of namespace launcherapp::commands::win32menu

