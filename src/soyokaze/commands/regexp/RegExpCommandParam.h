#pragma once

#include "hotkey/HotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace regexp {

struct ATTRIBUTE {

	ATTRIBUTE() : mShowType(SW_NORMAL) {}

	CString mPath;
	CString mParam;
	CString mDir;
	int mShowType;
};



class CommandParam
{
public:
	CString mName;
	CString mDescription;
	CString mPatternStr;
	int mRunAs = 0;

	ATTRIBUTE mNormalAttr;

	std::vector<uint8_t> mIconData;

};

}
}
}

