#include "pch.h"
#include "hotkey/HotKeyAttribute.h"
#include "hotkey/VirtualKeyDefine.h"
#include "setting/AppPreference.h"
#include <utility>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int ID_LAUNCHER_TRY_HOTKEY = 0xB31D;

// 修飾キー
// MOD_ALT     (0x0001)
// MOD_CONTROL (0x0002)
// MOD_SHIFT   (0x0004)
// MOD_WIN     (0x0008)
// MOD_NOREPEAT(0x4000)

HOTKEY_ATTR::HOTKEY_ATTR() : 
	mVirtualKeyIdx(-1), 
	mUseShift(false), mUseCtrl(false), mUseAlt(false), mUseWin(false)
{
}

HOTKEY_ATTR::HOTKEY_ATTR(const HOTKEY_ATTR& rhs) :
	mVirtualKeyIdx(rhs.mVirtualKeyIdx), 
	mUseShift(rhs.mUseShift), mUseCtrl(rhs.mUseCtrl), mUseAlt(rhs.mUseAlt), mUseWin(rhs.mUseWin)
{
}


HOTKEY_ATTR::HOTKEY_ATTR(UINT modifiers, UINT hotkey) : 
	mVirtualKeyIdx(-1)
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

	// modifiersのビットを見てわける
	mUseShift = (modifiers & MOD_SHIFT) != 0;
	mUseCtrl = (modifiers & MOD_CONTROL) != 0;
	mUseAlt = (modifiers & MOD_ALT) != 0;
	mUseWin = (modifiers & MOD_WIN) != 0;
}

bool HOTKEY_ATTR::operator == (const HOTKEY_ATTR& rhs) const
{
	if (mUseShift != rhs.mUseShift) {
		return false;
	}
	if (mUseCtrl != rhs.mUseCtrl) {
		return false;
	}
	if (mUseAlt != rhs.mUseAlt) {
		return false;
	}
	if (mUseWin != rhs.mUseWin) {
		return false;
	}
	if (mVirtualKeyIdx != rhs.mVirtualKeyIdx) {
		return false;
	}
	return true;
}

bool HOTKEY_ATTR::operator != (const HOTKEY_ATTR& rhs) const
{
	return !(*this == rhs);
}

bool HOTKEY_ATTR::operator < (const HOTKEY_ATTR& rhs) const
{
	if (mUseShift < rhs.mUseShift) {
		return true;
	}
	if (mUseShift > rhs.mUseShift) {
		return false;
	}
	if (mUseCtrl < rhs.mUseCtrl) {
		return true;
	}
	if (mUseCtrl > rhs.mUseCtrl) {
		return false;
	}
	if (mUseAlt < rhs.mUseAlt) {
		return true;
	}
	if (mUseAlt > rhs.mUseAlt) {
		return false;
	}
	if (mUseWin < rhs.mUseWin) {
		return true;
	}
	if (mUseWin > rhs.mUseWin) {
		return false;
	}
	if (mVirtualKeyIdx < rhs.mVirtualKeyIdx) {
		return true;
	}
	return false;
}

HOTKEY_ATTR& HOTKEY_ATTR::operator = (
	const HOTKEY_ATTR& rhs
)
{
	mUseShift = rhs.mUseShift;
	mUseCtrl = rhs.mUseCtrl;
	mUseAlt = rhs.mUseAlt;
	mUseWin = rhs.mUseWin;
	mVirtualKeyIdx = rhs.mVirtualKeyIdx;

	return *this;
}

bool HOTKEY_ATTR::IsValid() const
{
	return 0 <= mVirtualKeyIdx && mVirtualKeyIdx < VirtualKeyDefine::GetInstance()->GetItemCount();
}

bool HOTKEY_ATTR::GetAccel(ACCEL& accel) const
{
	if (IsValid() == false) {
		return false;
	}

	accel.cmd = 0;
	accel.fVirt = FVIRTKEY;
	if (mUseShift) {
		accel.fVirt |= FSHIFT;
	}
	if (mUseCtrl) {
		accel.fVirt |= FCONTROL;
	}
	if (mUseAlt) {
		accel.fVirt |= FALT;
	}

	auto keyDefine = VirtualKeyDefine::GetInstance();
	accel.key = (WORD)keyDefine->GetItem(mVirtualKeyIdx).mVKCode;

	return true;
}

/**
 *  ホットキーが登録可能かどうか調べる
 */
bool HOTKEY_ATTR::TryRegister(HWND targetWnd) const
{
	// アプリが使用中のキーの場合はtrueを返す
	auto pref = AppPreference::Get();
	if (pref->GetVirtualKeyCode() == GetVKCode() && pref->GetModifiers() == GetModifiers()) {
		return true;
	}

	// 登録できるか実際に試す(すぐに解除)
	if (RegisterHotKey(targetWnd, ID_LAUNCHER_TRY_HOTKEY, GetModifiers(), GetVKCode()) == FALSE) {
		return false;
	}
	UnregisterHotKey(targetWnd, ID_LAUNCHER_TRY_HOTKEY);
	return true;
}

CString HOTKEY_ATTR::ToString() const
{
	if (IsValid() == false) {
		return _T("");
	}

	CString str;
	if (mUseShift) {
		str += _T("Shift");
	}
	if (mUseAlt) {
		if (str.IsEmpty() == FALSE) {
			str += _T("-");
		}
		str += _T("Alt");
	}
	if (mUseCtrl) {
		if (str.IsEmpty() == FALSE) {
			str += _T("-");
		}
		str += _T("Ctrl");
	}
	if (mUseWin) {
		if (str.IsEmpty() == FALSE) {
			str += _T("-");
		}
		str += _T("Win");
	}

	if (str.IsEmpty() == FALSE) {
		str += _T("-");
	}

	auto keyDefine = VirtualKeyDefine::GetInstance();
	str += keyDefine->GetItem(mVirtualKeyIdx).mChar;
	return str;
}

UINT HOTKEY_ATTR::GetModifiers() const
{
	UINT modifiers = 0;
	if (mUseShift) {
		modifiers |= MOD_SHIFT;
	}
	if (mUseCtrl) {
		modifiers |= MOD_CONTROL;
	}
	if (mUseAlt) {
		modifiers |= MOD_ALT;
	}
	if (mUseWin) {
		modifiers |= MOD_WIN;
	}
	return modifiers;
}

UINT HOTKEY_ATTR::GetVKCode() const
{
	if (IsValid() == false) {
		return 0;
	}

	auto keyDefine = VirtualKeyDefine::GetInstance();
	return keyDefine->GetItem(mVirtualKeyIdx).mVKCode;
}

// 割り当てを許可しないキーか?
bool HOTKEY_ATTR::IsReservedKey() const
{
	auto keyDefine = VirtualKeyDefine::GetInstance();
	const auto& data = keyDefine->GetItem(mVirtualKeyIdx);
	auto kind = data.mKind;
	if (GetModifiers() == 0) {

		// 横取りすると通常の入力に差し支えある文字は許可しない
		return kind == VirtualKeyDefine::KIND_ALPHA || kind == VirtualKeyDefine::KIND_NUMBER ||
		       kind == VirtualKeyDefine::KIND_CHAR || kind == VirtualKeyDefine::KIND_MOVE;
	}
	if (GetModifiers() == MOD_SHIFT) {
		// Shift+CapsLockは許可しない
		if  (data.mVKCode == 0xF0)  {
			return true;
		}
		// Shift+英数字キーも差し支えありそうなので許可しない
		return kind == VirtualKeyDefine::KIND_ALPHA || kind == VirtualKeyDefine::KIND_NUMBER;
	}
	return false;
}

// 何にも割り当てられていないか
bool HOTKEY_ATTR::IsUnmapped() const
{
	return mUseShift == FALSE && mUseCtrl == FALSE && mUseAlt == FALSE && mUseWin == FALSE && 
	       mVirtualKeyIdx == -1;
}

void HOTKEY_ATTR::Reset()
{
	mVirtualKeyIdx = -1;
	mUseShift = false;
	mUseCtrl = false;
	mUseAlt = false;
	mUseWin = false;
}

