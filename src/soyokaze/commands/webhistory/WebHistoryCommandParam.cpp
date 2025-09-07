#include "pch.h"
#include "WebHistoryCommandParam.h"

namespace launcherapp { namespace commands { namespace webhistory {

bool CommandParam::Save(Settings& settings) const
{
	settings.Set(_T("WebHistory:Enable"), mIsEnable);
	settings.Set(_T("WebHistory:Prefix"), mPrefix);
	settings.Set(_T("WebHistory:Limit"), mLimit);
	settings.Set(_T("WebHistory:MinTriggerLength"), mMinTriggerLength);
	settings.Set(_T("WebHistory:EnableEdge"), mIsEnableEdge);
	settings.Set(_T("WebHistory:EnableChrome"), mIsEnableChrome);
	settings.Set(_T("WebHistory:UseMigemo"), mIsUseMigemo);
	settings.Set(_T("WebHistory:UseURL"), mIsUseURL);

	return true;
}

bool CommandParam::Load(Settings& settings)
{
	mIsEnable = settings.Get(_T("WebHistory:EnableBookmarks"), true);
	mPrefix = settings.Get(_T("WebHistory:Prefix"), _T(""));
	mLimit = settings.Get(_T("WebHistory:Limit"), 20);
	mMinTriggerLength = settings.Get(_T("WebHistory:MinTriggerLength"), 5);
	mIsEnableEdge = settings.Get(_T("WebHistory:EnableEdge"), true);
	mIsEnableChrome = settings.Get(_T("WebHistory:EnableChrome"), true);
	mIsUseMigemo = settings.Get(_T("WebHistory:UseMigemo"), true);
	mIsUseURL = settings.Get(_T("WebHistory:UseURL"), false);

	return true;
}

}}}

