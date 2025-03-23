#pragma once

#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace alias {

class CommandParam
{
public:
	CString mName;
	CString mDescription;
	CString mText;
	// 実行するかどうか(1:貼り付けのみ 0:実行する)
	int mIsPasteOnly = 0;
	CommandHotKeyAttribute mHotKeyAttr;
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

