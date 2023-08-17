#pragma once

#include "HotKeyAttribute.h"
#include <regex>

namespace soyokaze {
namespace commands {
namespace websearch {

class CommandParam
{
public:
	CommandParam();
	~CommandParam();

	bool IsEnableShortcutSearch() const;

public:
	CString mName;
	CString mDescription;

	CString mURL;

	bool mIsEnableShortcut;
};



}
}
}

