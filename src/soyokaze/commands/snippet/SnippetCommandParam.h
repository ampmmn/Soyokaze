#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include "commands/core/CommandEntryIF.h"

namespace launcherapp { namespace commands { namespace snippet {

class CommandParam
{
public:
	bool Save(CommandEntryIF* entry);
	bool Load(CommandEntryIF* entry);

	CString mName;
	CString mDescription;
	CString mText;
	CommandHotKeyAttribute mHotKeyAttr;
};

}}}

