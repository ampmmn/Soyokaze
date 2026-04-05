#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include "commands/core/CommandEntryIF.h"
#include <memory>
#include <regex>

namespace launcherapp { namespace commands { namespace place_window_in_region {

class CommandParam
{
public:
	CommandParam() = default;
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	bool IsValid(LPCTSTR orgName, int* errCode) const;

	bool Save(CommandEntryIF* entry) const;
	bool Load(CommandEntryIF* entry);

	CRect GetRegionRect() const;

public:
	CString mName;
	CString mDescription;


	WINDOWPLACEMENT mPlacement{};
	bool mIsActivate{true};

	CommandHotKeyAttribute mHotKeyAttr;
};



}}}

