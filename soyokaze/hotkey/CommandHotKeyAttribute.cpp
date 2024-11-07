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
	mHotKeyAttr(rhs.mHotKeyAttr), mSandSKeyAttr(rhs.mSandSKeyAttr), mIsGlobal(rhs.mIsGlobal)
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
	if (mSandSKeyAttr != rhs.mSandSKeyAttr) {
		return false;
	}


	return mIsGlobal == rhs.mIsGlobal;
}

bool CommandHotKeyAttribute::operator != (const CommandHotKeyAttribute& rhs) const
{
	if (mHotKeyAttr != rhs.mHotKeyAttr) {
		return true;
	}
	if (mSandSKeyAttr != rhs.mSandSKeyAttr) {
		return true;
	}

	return mIsGlobal != rhs.mIsGlobal;
}

bool CommandHotKeyAttribute::operator < (const CommandHotKeyAttribute& rhs) const
{
	if (mHotKeyAttr < rhs.mHotKeyAttr) {
		return true;
	}
	if (mSandSKeyAttr < rhs.mSandSKeyAttr) {
		return true;
	}
	return mIsGlobal < rhs.mIsGlobal;
}


CommandHotKeyAttribute& 
CommandHotKeyAttribute::operator = (const CommandHotKeyAttribute& rhs)
{
	if (this != &rhs) {
		mHotKeyAttr = rhs.mHotKeyAttr;
		mSandSKeyAttr = rhs.mSandSKeyAttr;
		mIsGlobal = rhs.mIsGlobal;
	}
	return *this;
}

void CommandHotKeyAttribute::SetSandsHotKey(UINT modifier, UINT vk)
{
	mSandSKeyAttr = SANDSKEY_ATTR(modifier, vk);
}

void CommandHotKeyAttribute::Reset()
{
	mHotKeyAttr.Reset();
}

void CommandHotKeyAttribute::ResetSandS()
{
	mSandSKeyAttr.Reset();
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

bool CommandHotKeyAttribute::IsValidSandS() const
{
	return mSandSKeyAttr.IsValid();
}

UINT CommandHotKeyAttribute::GetSandSModifier() const
{
	return mSandSKeyAttr.GetModifier();
}

UINT CommandHotKeyAttribute::GetSandSVKCode() const
{
	return mSandSKeyAttr.GetVKCode();
}


CString CommandHotKeyAttribute::ToString() const
{
	CString str;

	bool hasSandS = mSandSKeyAttr.IsValid();
	if (mHotKeyAttr.IsValid()) {
		str = mHotKeyAttr.ToString();
		if (hasSandS) {
			str += _T("/");
		}
	}
	if (hasSandS) {
		str += mSandSKeyAttr.ToString();
	}

	return str;
}
