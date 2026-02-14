#include "pch.h"
#include "UIAutomationParam.h"
#include "setting/Settings.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace uiautomation {

CommandParam::CommandParam()
{
}


CommandParam::CommandParam(const CommandParam& rhs)
{
	mIsEnable = rhs.mIsEnable;
	mPrefix = rhs.mPrefix;
	mIsEnableMenuItem = rhs.mIsEnableMenuItem;
	mIsEnableTabPages = rhs.mIsEnableTabPages;
	mIsDebugDumpEnabled = rhs.mIsDebugDumpEnabled;
}

CommandParam::~CommandParam()
{
}

CommandParam& CommandParam::operator = (const CommandParam& rhs)
{
	if (&rhs != this) {
		mIsEnable = rhs.mIsEnable;
		mPrefix = rhs.mPrefix;
		mIsEnableMenuItem = rhs.mIsEnableMenuItem;
		mIsEnableTabPages = rhs.mIsEnableTabPages;
		mIsDebugDumpEnabled = rhs.mIsDebugDumpEnabled;
	}
	return *this;
}

bool CommandParam::Save(Settings& settings) const
{
	settings.Set(_T("UIAutomationSetting:IsEnable"), mIsEnable);
	settings.Set(_T("UIAutomationSetting:Prefix"), mPrefix);
	settings.Set(_T("UIAutomationSetting:IsEnableMenuItem"), mIsEnableMenuItem);
	settings.Set(_T("UIAutomationSetting:IsEnableTabPages"), mIsEnableTabPages);
	settings.Set(_T("UIAutomationSetting:IsDebugDumpEnabled"), mIsDebugDumpEnabled);
	return true;
}

bool CommandParam::Load(Settings& settings)
{
	mIsEnable = settings.Get(_T("UIAutomationSetting:IsEnable"), false);
	mPrefix = settings.Get(_T("UIAutomationSetting:Prefix"), _T("ui"));
	mIsEnableMenuItem = settings.Get(_T("UIAutomationSetting:IsEnableMenuItem"), false);
	mIsEnableTabPages = settings.Get(_T("UIAutomationSetting:IsEnableTabPages"), false);
	mIsDebugDumpEnabled = settings.Get(_T("UIAutomationSetting:IsDebugDumpEnabled"), false);
	return true;
}

}}} // end of namespace launcherapp::commands::uiautomation

