#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include "commands/core/CommandEntryIF.h"

namespace launcherapp { namespace commands { namespace ejectvolume {

class CommandParam
{
public:
	CommandParam() : mDriveLetter(_T('A')) {}
	CommandParam(const CommandParam&) = default;
	~CommandParam() {}

	CommandParam& operator = (const CommandParam&) = default;

	bool Save(CommandEntryIF* entry);
	bool Load(CommandEntryIF* entry);

public:
	CString mName;
	CString mDescription;

	// ドライブレター('A'～'Z')
	TCHAR mDriveLetter;
	CommandHotKeyAttribute mHotKeyAttr;
};


}}}

