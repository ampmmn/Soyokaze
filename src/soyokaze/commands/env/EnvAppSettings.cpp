#include "pch.h"
#include "EnvAppSettings.h"
#include "setting/AppPreference.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace env {

void AppSettings::Load()
{
	auto pref = AppPreference::Get();
	auto& settings = const_cast<Settings&>(pref->GetSettings());

	mPrefix = settings.Get(_T("Environment:Prefix"), _T("env"));
	mIsEnable = settings.Get(_T("Environment:IsEnable"), true);
}


}}} // end of namespace launcherapp::commands::env

