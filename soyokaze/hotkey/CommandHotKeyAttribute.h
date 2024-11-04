#pragma once

#include "hotkey/HotKeyAttribute.h"

// コマンド用のホットキー属性
class CommandHotKeyAttribute
{
public:
	CommandHotKeyAttribute(bool isGlobal = false);
	CommandHotKeyAttribute(const CommandHotKeyAttribute& rhs);
	CommandHotKeyAttribute(UINT modifiers, UINT hotkey, bool isGlobal = false);

	bool operator == (const CommandHotKeyAttribute& rhs) const;
	bool operator != (const CommandHotKeyAttribute& rhs) const;
	bool operator < (const CommandHotKeyAttribute& rhs) const;

	CommandHotKeyAttribute& operator = (const CommandHotKeyAttribute& rhs);

	void Reset();

	bool IsValid() const;
	UINT GetVKCode() const;
	UINT GetModifiers() const;
	bool IsGlobal() const;

	CString ToString() const;

	HOTKEY_ATTR mHotKeyAttr;
	bool mIsGlobal;
};



