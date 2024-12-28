#pragma once

#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace snippet {

class CommandParam
{
public:
	CString mName;
	CString mDescription;
	CString mText;
	CommandHotKeyAttribute mHotKeyAttr;
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

