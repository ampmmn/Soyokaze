#include "pch.h"
#include "hotkey/SandSKeyAttribute.h"
#include "hotkey/VirtualKeyDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SANDSKEY_ATTR::SANDSKEY_ATTR() : 
	mModifier(MOD_UNINITIALIZED), mVirtualKeyIdx(-1)
{
}

SANDSKEY_ATTR::SANDSKEY_ATTR(const SANDSKEY_ATTR& rhs) :
	mModifier(rhs.mModifier), mVirtualKeyIdx(rhs.mVirtualKeyIdx)
{
}


SANDSKEY_ATTR::SANDSKEY_ATTR(UINT modifier, UINT hotkey) : 
	mModifier((short)modifier), mVirtualKeyIdx(-1)
{
	auto keyDefine = VirtualKeyDefine::GetInstance();
	int count = keyDefine->GetItemCount();
	for (int i = 0; i < count; ++i) {
		auto key = keyDefine->GetItem(i);
		if (key.mVKCode == hotkey) {
			mVirtualKeyIdx = (short)i;
			break;
		}
	}
}

bool SANDSKEY_ATTR::operator == (const SANDSKEY_ATTR& rhs) const
{
	if (mModifier != rhs.mModifier) {
		return false;
	}
	return (mVirtualKeyIdx == rhs.mVirtualKeyIdx);
}

bool SANDSKEY_ATTR::operator != (const SANDSKEY_ATTR& rhs) const
{
	return !(*this == rhs);
}

bool SANDSKEY_ATTR::operator < (const SANDSKEY_ATTR& rhs) const
{
	if (mModifier < rhs.mModifier) {
		return true;
	}
	if (mModifier > rhs.mModifier) {
		return false;
	}
	return (mVirtualKeyIdx < rhs.mVirtualKeyIdx);
}

SANDSKEY_ATTR& SANDSKEY_ATTR::operator = (
	const SANDSKEY_ATTR& rhs
)
{
	mModifier = rhs.mModifier;
	mVirtualKeyIdx = rhs.mVirtualKeyIdx;

	return *this;
}

bool SANDSKEY_ATTR::IsValid() const
{
	return mModifier != MOD_UNINITIALIZED &&
	       0 <= mVirtualKeyIdx && mVirtualKeyIdx < VirtualKeyDefine::GetInstance()->GetItemCount();
}

CString SANDSKEY_ATTR::ToString() const
{
	if (IsValid() == false) {
		return _T("");
	}

	CString str;
	if (mModifier == MOD_SPACE) {
		str += _T("Space");
	}
	else if (mModifier == MOD_CAPSLOCK) {
		str += _T("CapsLock");
	}
	else if (mModifier == MOD_CONVERT) {
		str += _T("変換");
	}
	else if (mModifier == MOD_NONCONVERT) {
		str += _T("無変換");
	}
	else if (mModifier == MOD_KANA) {
		str += _T("かな");
	}
	if (str.IsEmpty() == FALSE) {
		str += _T("-");
	}

	auto keyDefine = VirtualKeyDefine::GetInstance();
	str += keyDefine->GetItem(mVirtualKeyIdx).mChar;
	return str;
}

UINT SANDSKEY_ATTR::GetModifier() const
{
	return mModifier;
}

UINT SANDSKEY_ATTR::GetVKCode() const
{
	if (IsValid() == false) {
		return 0;
	}

	auto keyDefine = VirtualKeyDefine::GetInstance();
	return keyDefine->GetItem(mVirtualKeyIdx).mVKCode;
}

UINT SANDSKEY_ATTR::GetModifierVKCode() const
{
	// FIXME: VirtualKeyDefineから取得する
	if (mModifier == MOD_SPACE) {
		return 0x20;
	}
	else if (mModifier == MOD_CAPSLOCK) {
		return 0xF0;
	}
	else if (mModifier == MOD_CONVERT) {
		return 0x1C;
	}
	else if (mModifier == MOD_NONCONVERT) {
		return 0x1D;
	}
	else if (mModifier == MOD_KANA) {
		return 0xF2;
	}
	else {
		return (UINT)-1;
	}
}

// 何にも割り当てられていないか
bool SANDSKEY_ATTR::IsUnmapped() const
{
	return mModifier == MOD_UNINITIALIZED || mVirtualKeyIdx == -1;
}

void SANDSKEY_ATTR::Reset()
{
	mModifier = MOD_UNINITIALIZED;
	mVirtualKeyIdx = -1;
}

