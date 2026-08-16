#include "pch.h"
#include "CurrencyCommandParam.h"
#include "setting/Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace currency {

bool CommandParam::Save(Settings& settings) const
{
	settings.Set(_T("Currency:IsEnable"), mIsEnable);
	return true;
}

bool CommandParam::Load(Settings& settings)
{
	mIsEnable = settings.Get(_T("Currency:IsEnable"), false);
	return true;
}


}}}
