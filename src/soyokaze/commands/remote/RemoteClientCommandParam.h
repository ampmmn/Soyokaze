#pragma once

#include "commands/core/CommandEntryIF.h"
#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp { namespace commands { namespace remote {

class CommandParam
{
public:
	bool Save(CommandEntryIF* entry)
	{
		entry->Set(_T("Description"), mDescription);
		entry->Set(_T("Host"), mHost);
		entry->Set(_T("LauncherAppDir"), mLauncherAppDir);
		return true;
	}

	bool Load(CommandEntryIF* entry)
	{
		mName = entry->GetName();
		mDescription = entry->Get(_T("Description"), _T(""));
		mHost = entry->Get(_T("Host"), _T(""));
		mLauncherAppDir = entry->Get(_T("LauncherAppDir"), _T(""));

		return true;
	}

public:
	CString mName;
	CString mDescription;
	CString mHost;
	CString mLauncherAppDir;
	CommandHotKeyAttribute mHotKeyAttr;
};

}}} // end of namespace launcherapp::commands::remote
