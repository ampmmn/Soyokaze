#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include "commands/core/CommandEntryIF.h"

namespace launcherapp { namespace commands { namespace alias {

class CommandParam
{
public:
	bool Save(CommandEntryIF* entry) const;
	bool Load(CommandEntryIF* entry);

	bool IsValid(LPCTSTR orgName, int* errCode) const;

public:
	CString mName;
	CString mDescription;
	CString mText;
	// 実行するかどうか(1:貼り付けのみ 0:実行する)
	int mIsPasteOnly{0};
	// 自動実行を許可するか?
	bool mIsAllowAutoExecute{false};
	CommandHotKeyAttribute mHotKeyAttr;
};

}}} // end of namespace launcherapp::commands::builtin

