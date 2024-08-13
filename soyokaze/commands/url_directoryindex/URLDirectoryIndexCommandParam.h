#pragma once

#include "hotkey/CommandHotkeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace url_directoryindex {

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	CString CombineURL(const CString& subPath) const;

	bool IsContentURL(const CString& url) const;

public:
	CString mName;
	CString mDescription;
	// ベースURL
	CString mURL;

	CommandHotKeyAttribute mHotKeyAttr;
};

} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp

