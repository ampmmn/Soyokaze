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

	// パラメータを渡すか
	bool mIsPassParam;
	// 繰り返し実行するか
	bool mIsRepeat;
	// 繰り返し実行する場合の繰り返し数(繰り返さない場合は1)
	int mRepeats;
	// 確認ダイアログを出す
	bool mIsConfirm;
	// 自動実行を許可するか?
	bool mIsAllowAutoExecute;

	CommandHotKeyAttribute mHotKeyAttr;
};



} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp

