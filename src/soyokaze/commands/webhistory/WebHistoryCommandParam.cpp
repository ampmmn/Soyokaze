#include "pch.h"
#include "WebHistoryCommandParam.h"

namespace launcherapp { namespace commands { namespace webhistory {

bool CommandParam::Save(Settings& settings) const
{
	// Note: AppPreferenceにもGetterがあるのでキーを合わせること!
	settings.Set(_T("Browser:EnableWebHistory"), mIsEnable);
	settings.Set(_T("Browser:PrefixWebHistory"), mPrefix);
	settings.Set(_T("Browser:Candidates"), mLimit);
	settings.Set(_T("Browser:WebHistoryMinTriggerLength"), mMinTriggerLength);
	settings.Set(_T("Browser:EnableHistoryEdge"), mIsEnableEdge);
	settings.Set(_T("Browser:EnableHistoryChrome"), mIsEnableChrome);
	settings.Set(_T("Browser:UseMigemo"), mIsUseMigemo);
	settings.Set(_T("Browser:UseURL"), mIsUseURL);

	return true;
}

bool CommandParam::Load(Settings& settings)
{
	mIsEnable = settings.Get(_T("Browser:EnableWebHistory"), true);
	mPrefix = settings.Get(_T("Browser:PrefixWebHistory"), _T(""));
	mLimit = settings.Get(_T("Browser:Candidates"), 20);
	mMinTriggerLength = settings.Get(_T("Browser:WebHistoryMinTriggerLength"), 5);
	mIsEnableEdge = settings.Get(_T("Browser:EnableHistoryEdge"), true);
	mIsEnableChrome = settings.Get(_T("Browser:EnableHistoryChrome"), true);
	mIsUseMigemo = settings.Get(_T("Browser:UseMigemo"), true);
	mIsUseURL = settings.Get(_T("Browser:UseURL"), false);

	return true;
}

}}}

