// あ
#pragma once

#include "hotkey/HotKeyAttribute.h"
#include "hotkey/SandSKeyAttribute.h"

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

	void SetSandsHotKey(UINT modifier, UINT vk);

	void Reset();
	void ResetSandS();

	bool IsValid() const;
	UINT GetVKCode() const;
	UINT GetModifiers() const;
	bool IsGlobal() const;

	bool IsValidSandS() const;
	UINT GetSandSModifier() const;
	UINT GetSandSVKCode() const;

	CString ToString() const;

	HOTKEY_ATTR mHotKeyAttr;
	SANDSKEY_ATTR mSandSKeyAttr;
	bool mIsGlobal;
};



