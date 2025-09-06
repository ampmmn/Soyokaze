#pragma once

#include "commands/core/CommandEntryIF.h"
#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp { namespace commands { namespace bookmarks {

class CommandParam
{
public:
	CommandParam();
	~CommandParam();

	void Save(CommandEntryIF* entry);
	void Load(CommandEntryIF* entry);

public:
	CString mName;
	CString mDescription;
	bool mIsEnableChrome;
	bool mIsEnableEdge;
	bool mIsUseURL;
	CommandHotKeyAttribute mHotKeyAttr;
};


}}} // end of namespace launcherapp::commands::bookmarks

