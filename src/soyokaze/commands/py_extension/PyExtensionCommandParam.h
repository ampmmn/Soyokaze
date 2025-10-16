#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include "commands/core/CommandEntryIF.h"

namespace launcherapp { namespace commands { namespace py_extension {

class CommandParam
{
public:
	bool Save(CommandEntryIF* entry) const;
	bool Load(CommandEntryIF* entry);

	bool IsValid(LPCTSTR orgName, int* errCode) const;

public:
	CString mName;
	CString mDescription;
	CString mScript;
	CString mWorkDir;
	CommandHotKeyAttribute mHotKeyAttr;
};

}}} // end of namespace launcherapp::commands::py_extension

