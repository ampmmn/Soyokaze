#include "pch.h"
#include "BookmarkCommandParam.h"
#include "resource.h"

namespace launcherapp { namespace commands { namespace bookmarks {

CommandParam::CommandParam() : 
	mIsEnableEdge(true), mIsEnableChrome(true), mIsUseURL(false)
{
}

CommandParam::~CommandParam()
{
}

void CommandParam::Save(CommandEntryIF* entry)
{
	entry->Set(_T("description"), mDescription);
	entry->Set(_T("EnableChrome"), mIsEnableChrome);
	entry->Set(_T("EnableEdge"), mIsEnableEdge);
	entry->Set(_T("UseURL"), mIsUseURL);
}

void CommandParam::Load(CommandEntryIF* entry)
{
	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));

	mIsEnableChrome = entry->Get(_T("EnableChrome"), true);
	mIsEnableEdge = entry->Get(_T("EnableEdge"), true);
	mIsUseURL = entry->Get(_T("UseURL"), false);
}

}}} // end of namespace launcherapp::commands::bookmarks

