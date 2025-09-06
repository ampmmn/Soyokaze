#include "pch.h"
#include "BookmarkCommandParam.h"

namespace launcherapp { namespace commands { namespace bookmarks {

bool CommandParam::Save(Settings& settings) const
{
	settings.Set(_T("Bookmarks:EnableBookmarks"), mIsEnable);
	settings.Set(_T("Bookmarks:Prefix"), mPrefix);
	settings.Set(_T("Bookmarks:MinTriggerLength"), mMinTriggerLength);
	settings.Set(_T("Bookmarks:EnableChrome"), mIsEnableChrome);
	settings.Set(_T("Bookmarks:EnableEdge"), mIsEnableEdge);
	settings.Set(_T("Bookmarks:UseURL"), mIsUseURL);

	return true;
}

bool CommandParam::Load(Settings& settings)
{
	mIsEnable = settings.Get(_T("Bookmarks:EnableBookmarks"), true);
	mPrefix = settings.Get(_T("Bookmarks:Prefix"), _T(""));
	mMinTriggerLength = settings.Get(_T("Bookmarks:MinTriggerLength"), 5);
	mIsEnableChrome = settings.Get(_T("Bookmarks:EnableChrome"), true);
	mIsEnableEdge = settings.Get(_T("Bookmarks:EnableEdge"), true);
	mIsUseURL = settings.Get(_T("Bookmarks:UseURL"), false);

	return true;
}

}}} // end of namespace launcherapp::commands::bookmarks

