#pragma once

#include "hotkey/HotKeyAttribute.h"

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

	HOTKEY_ATTR mHotKeyAttr;
	bool mIsGlobal;

};


}
}
}

