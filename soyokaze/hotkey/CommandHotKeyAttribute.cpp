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
	mHotKeyAttr(rhs.mHotKeyAttr), mIsGlobal(rhs.mIsGlobal)
{
}

CommandHotKeyAttribute::CommandHotKeyAttribute(UINT modifiers, UINT hotkey, bool isGlobal) :
	mHotKeyAttr(modifiers, hotkey), mIsGlobal(isGlobal)
{
}


bool CommandHotKeyAttribute::operator == (const CommandHotKeyAttribute& rhs) const
{
	if (mHotKeyAttr != rhs.mHotKeyAttr) {
		return false;
	}
	return mIsGlobal == rhs.mIsGlobal;
}

bool CommandHotKeyAttribute::operator != (const CommandHotKeyAttribute& rhs) const
{
	if (mHotKeyAttr != rhs.mHotKeyAttr) {
		return true;
	}
	return mIsGlobal != rhs.mIsGlobal;
}

bool CommandHotKeyAttribute::operator < (const CommandHotKeyAttribute& rhs) const
{
	if (mHotKeyAttr < rhs.mHotKeyAttr) {
		return true;
	}
	return mIsGlobal < rhs.mIsGlobal;
}


CommandHotKeyAttribute& 
CommandHotKeyAttribute::operator = (const CommandHotKeyAttribute& rhs)
{
	if (this != &rhs) {
		mHotKeyAttr = rhs.mHotKeyAttr;
		mIsGlobal = rhs.mIsGlobal;
	}
	return *this;
}

void CommandHotKeyAttribute::Reset()
{
	mHotKeyAttr.Reset();
}



bool CommandHotKeyAttribute::IsValid() const
{
	return mHotKeyAttr.IsValid();
}

UINT CommandHotKeyAttribute::GetVKCode() const
{
	return mHotKeyAttr.GetVKCode();
}

UINT CommandHotKeyAttribute::GetModifiers() const
{
	return mHotKeyAttr.GetModifiers();
}


bool CommandHotKeyAttribute::IsGlobal() const
{
	return mIsGlobal;
}

CString CommandHotKeyAttribute::ToString() const
{
	return mHotKeyAttr.ToString();
}
