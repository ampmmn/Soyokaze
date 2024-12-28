#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include <vector>

namespace launcherapp {
namespace commands {
namespace group {

struct GroupItem
{
	CString mItemName;
	bool mIsWait = false;

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

	// パラメータを渡すか
	BOOL mIsPassParam;
	// 繰り返し実行するか
	BOOL mIsRepeat;
	// 繰り返し実行する場合の繰り返し数(繰り返さない場合は1)
	int mRepeats;
	// 確認ダイアログを出す
	BOOL mIsConfirm;

	CommandHotKeyAttribute mHotKeyAttr;
};



} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp

