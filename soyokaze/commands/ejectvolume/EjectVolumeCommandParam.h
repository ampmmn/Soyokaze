#pragma once

#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace ejectvolume {

class CommandParam
{
public:
	CommandParam() {}
	~CommandParam() {}

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

