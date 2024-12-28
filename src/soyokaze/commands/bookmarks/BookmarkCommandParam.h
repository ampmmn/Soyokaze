#pragma once

#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace bookmarks {

class CommandParam
{
public:
	CommandParam();
	~CommandParam();

public:
	CString mName;
	CString mDescription;
	bool mIsEnableChrome;
	bool mIsEnableEdge;
	bool mIsUseURL;
	CommandHotKeyAttribute mHotKeyAttr;
};



}
}
}

