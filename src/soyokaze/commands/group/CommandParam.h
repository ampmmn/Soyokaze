#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include "commands/core/CommandEntryIF.h"
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

	bool Save(CommandEntryIF* entry) const;
	bool Load(CommandEntryIF* entry);

public:
	CString mName;
	CString mDescription;
	std::vector<GroupItem> mItems;

	// $B%Q%i%a!<%?$rEO$9$+(B
	bool mIsPassParam;
	// $B7+$jJV$7<B9T$9$k$+(B
	bool mIsRepeat;
	// $B7+$jJV$7<B9T$9$k>l9g$N7+$jJV$7?t(B($B7+$jJV$5$J$$>l9g$O(B1)
	int mRepeats;
	// $B3NG'%@%$%"%m%0$r=P$9(B
	bool mIsConfirm;
	// $B<+F0<B9T$r5v2D$9$k$+(B?
	bool mIsAllowAutoExecute;

	CommandHotKeyAttribute mHotKeyAttr;
};



} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp

