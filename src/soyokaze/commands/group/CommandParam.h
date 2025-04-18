#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include <vector>

namespace launcherapp {
namespace commands {
namespace group {

struct GroupItem
{
	CString mItemName;
	bool mIsWait{false};

};

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

public:
	CString mName;
	CString mDescription;
	std::vector<GroupItem> mItems;

	// $B%Q%i%a!<%?$rEO$9$+(B
	BOOL mIsPassParam;
	// $B7+$jJV$7<B9T$9$k$+(B
	BOOL mIsRepeat;
	// $B7+$jJV$7<B9T$9$k>l9g$N7+$jJV$7?t(B($B7+$jJV$5$J$$>l9g$O(B1)
	int mRepeats;
	// $B3NG'%@%$%"%m%0$r=P$9(B
	BOOL mIsConfirm;

	CommandHotKeyAttribute mHotKeyAttr;
};



} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp

