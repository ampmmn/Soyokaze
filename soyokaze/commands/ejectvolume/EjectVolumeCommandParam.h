#pragma once

#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace ejectvolume {

class CommandParam
{
public:
	CommandParam() : mDriveLetter(_T('A')) {}
	CommandParam(const CommandParam&) = default;
	~CommandParam() {}

	CommandParam& operator = (const CommandParam&) = default;

public:
	CString mName;
	CString mDescription;

	// ドライブレター('A'～'Z')
	TCHAR mDriveLetter;
	CommandHotKeyAttribute mHotKeyAttr;
};


}
}
}

