#pragma once

#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace webhistory {

class CommandParam
{
public:
	CommandParam();
	~CommandParam();

public:
	CString mName;
	CString mDescription;
	CString mKeyword;
	int mTimeout;
	int mLimit;
	bool mIsEnableHistoryEdge;
	bool mIsEnableHistoryChrome;
	bool mIsUseMigemo;
	bool mIsUseURL;
	CommandHotKeyAttribute mHotKeyAttr;
};



}
}
}

