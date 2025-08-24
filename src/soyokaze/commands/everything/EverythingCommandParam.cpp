#include "pch.h"
#include "EverythingCommandParam.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace everything {

CommandParam::CommandParam()
{
}


CommandParam::~CommandParam()
{
}

CommandParam& CommandParam::operator = (const CommandParam& rhs)
{
	if (this != &rhs) {
		mPrefix = rhs.mPrefix;
		mIsEnable = rhs.mIsEnable;
		mIsRunApp = rhs.mIsRunApp;
		mEverythingExePath = rhs.mEverythingExePath;
	}
	return *this;
}

bool CommandParam::Save(Settings& settings) const
{
	settings.Set(_T("Everything:Prefix"), mPrefix);
	settings.Set(_T("Everything:IsEnable"), mIsEnable);
	settings.Set(_T("Everything:IsRunApp"), mIsRunApp);
	settings.Set(_T("Everything:EverythingExePath"), mEverythingExePath);
	return true;
}

bool CommandParam::Load(Settings& settings)
{
	mPrefix = settings.Get(_T("Everything:Prefix"), _T(""));
	mIsEnable = settings.Get(_T("Everything:IsEnable"), true);
	mIsRunApp = settings.Get(_T("Everything:IsRunApp"), false);
	mEverythingExePath = settings.Get(_T("Everything:EverythingExePath"), _T(""));

	return true;
}

}}} // end of namespace launcherapp::commands::everything

