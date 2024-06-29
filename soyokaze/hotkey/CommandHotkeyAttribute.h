#pragma once

#include "hotkey/HotKeyAttribute.h"

// コマンド用のホットキー属性
class CommandHotKeyAttribute : public HOTKEY_ATTR
{
public:
	CommandHotKeyAttribute(bool isGlobal = false);
	CommandHotKeyAttribute(const CommandHotKeyAttribute& rhs);
	CommandHotKeyAttribute(UINT modifiers, UINT hotkey, bool isGlobal = false);

	bool operator == (const CommandHotKeyAttribute& rhs) const;
	bool operator != (const CommandHotKeyAttribute& rhs) const;
	bool operator < (const CommandHotKeyAttribute& rhs) const;

	CommandHotKeyAttribute& operator = (const CommandHotKeyAttribute& rhs);

	bool IsGlobal() const;

	bool mIsGlobal;
};



