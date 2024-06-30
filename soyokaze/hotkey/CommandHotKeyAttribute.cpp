#include "pch.h"
#include "CommandHotKeyAttribute.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CommandHotKeyAttribute::CommandHotKeyAttribute(bool isGlobal) : 
	mIsGlobal(isGlobal)
{
}

CommandHotKeyAttribute::CommandHotKeyAttribute(const CommandHotKeyAttribute& rhs) : 
	HOTKEY_ATTR(rhs), mIsGlobal(rhs.mIsGlobal)
{
}

CommandHotKeyAttribute::CommandHotKeyAttribute(UINT modifiers, UINT hotkey, bool isGlobal) :
	HOTKEY_ATTR(modifiers, hotkey), mIsGlobal(isGlobal)
{
}


bool CommandHotKeyAttribute::operator == (const CommandHotKeyAttribute& rhs) const
{
	if (!(*(HOTKEY_ATTR*)this == *(HOTKEY_ATTR*)&rhs)) {
		return false;
	}
	return mIsGlobal == rhs.mIsGlobal;
}

bool CommandHotKeyAttribute::operator != (const CommandHotKeyAttribute& rhs) const
{
	if ((*(HOTKEY_ATTR*)this != *(HOTKEY_ATTR*)&rhs)) {
		return true;
	}
	return mIsGlobal != rhs.mIsGlobal;
}

bool CommandHotKeyAttribute::operator < (const CommandHotKeyAttribute& rhs) const
{
	if ((*(HOTKEY_ATTR*)this < *(HOTKEY_ATTR*)&rhs)) {
		return true;
	}
	return mIsGlobal < rhs.mIsGlobal;
}


CommandHotKeyAttribute& 
CommandHotKeyAttribute::operator = (const CommandHotKeyAttribute& rhs)
{
	if (this != &rhs) {
		((HOTKEY_ATTR*)this)->operator = (rhs);
		mIsGlobal = rhs.mIsGlobal;
	}
	return *this;
}


bool CommandHotKeyAttribute::IsGlobal() const
{
	return mIsGlobal;
}

