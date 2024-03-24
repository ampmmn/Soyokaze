#include "pch.h"
#include "hotkey/HotKeyAttribute.h"
#include "setting/AppPreference.h"
#include <utility>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int ID_SOYOKAZE_TRY_HOTKEY = 0xB31D;

enum {
	KIND_ALPHA,
	KIND_NUMBER,
	KIND_NUMKEY,
	KIND_FUNCTION,
	KIND_OTHER,
};

struct HOTKEY_ATTR::VK_ITEM
{
	VK_ITEM(UINT vk, LPCTSTR chr, int kind) : 
		mVKCode(vk), mChar(chr), mKind(kind)
	{
	}
	VK_ITEM(const VK_ITEM&) = default;
	VK_ITEM& operator = (const VK_ITEM&) = default;

	UINT mVKCode;
	CString mChar;
	int mKind;
};

const HOTKEY_ATTR::VK_ITEM HOTKEY_ATTR::VK_DEFINED_DATA[] = {
	{ 0x41, _T("A"), KIND_ALPHA },
	{ 0x42, _T("B"), KIND_ALPHA },
	{ 0x43, _T("C"), KIND_ALPHA },
	{ 0x44, _T("D"), KIND_ALPHA },
	{ 0x45, _T("E"), KIND_ALPHA },
	{ 0x46, _T("F"), KIND_ALPHA },
	{ 0x47, _T("G"), KIND_ALPHA },
	{ 0x48, _T("H"), KIND_ALPHA },
	{ 0x49, _T("I"), KIND_ALPHA },
	{ 0x4A, _T("J"), KIND_ALPHA },
	{ 0x4B, _T("K"), KIND_ALPHA },
	{ 0x4C, _T("L"), KIND_ALPHA },
	{ 0x4D, _T("M"), KIND_ALPHA },
	{ 0x4E, _T("N"), KIND_ALPHA },
	{ 0x4F, _T("O"), KIND_ALPHA },
	{ 0x50, _T("P"), KIND_ALPHA },
	{ 0x51, _T("Q"), KIND_ALPHA },
	{ 0x52, _T("R"), KIND_ALPHA },
	{ 0x53, _T("S"), KIND_ALPHA },
	{ 0x54, _T("T"), KIND_ALPHA },
	{ 0x55, _T("U"), KIND_ALPHA },
	{ 0x56, _T("V"), KIND_ALPHA },
	{ 0x57, _T("W"), KIND_ALPHA },
	{ 0x58, _T("X"), KIND_ALPHA },
	{ 0x59, _T("Y"), KIND_ALPHA },
	{ 0x5A, _T("Z"), KIND_ALPHA },
	{ 0x31, _T("1"), KIND_NUMBER },
	{ 0x32, _T("2"), KIND_NUMBER },
	{ 0x33, _T("3"), KIND_NUMBER },
	{ 0x34, _T("4"), KIND_NUMBER },
	{ 0x35, _T("5"), KIND_NUMBER },
	{ 0x36, _T("6"), KIND_NUMBER },
	{ 0x37, _T("7"), KIND_NUMBER },
	{ 0x38, _T("8"), KIND_NUMBER },
	{ 0x39, _T("9"), KIND_NUMBER },
	{ 0x30, _T("0"), KIND_NUMBER },
	{ 0x20, _T("Space"), KIND_OTHER },
	{ 0x0D, _T("Enter"), KIND_OTHER },
	{ 0x2E, _T("Delete"), KIND_OTHER },
	{ 0x09, _T("Tab"), KIND_OTHER },
	{ 0x26, _T("Up"), KIND_OTHER },
	{ 0x28, _T("Down"), KIND_OTHER },
	{ 0x25, _T("Left"), KIND_OTHER },
	{ 0x27, _T("Right"), KIND_OTHER },
	{ 0x6C, _T(","), KIND_OTHER },
	{ 0x6E, _T("."), KIND_OTHER },
	{ 0x6F, _T("/"), KIND_OTHER },
	{ 0xBA, _T(":"), KIND_OTHER },
	{ 0xBB, _T(";"), KIND_OTHER },
	{ 0xC0, _T("@"), KIND_OTHER },
	{ 0xDB, _T("["), KIND_OTHER },
	{ 0xDD, _T("]"), KIND_OTHER },
	{ 0xBD, _T("^"), KIND_OTHER },
	{ 0x6D, _T("-"), KIND_OTHER },
	{ 0x22, _T("PageDown"), KIND_OTHER },
	{ 0x21, _T("PageUp"), KIND_OTHER },
	{ 0x24, _T("Home"), KIND_OTHER },
	{ 0x23, _T("End"), KIND_OTHER },
	{ 0x2D, _T("Insert"), KIND_OTHER },
	{ 0x61, _T("Num 1"), KIND_NUMKEY },
	{ 0x62, _T("Num 2"), KIND_NUMKEY },
	{ 0x63, _T("Num 3"), KIND_NUMKEY },
	{ 0x64, _T("Num 4"), KIND_NUMKEY },
	{ 0x65, _T("Num 5"), KIND_NUMKEY },
	{ 0x66, _T("Num 6"), KIND_NUMKEY },
	{ 0x67, _T("Num 7"), KIND_NUMKEY },
	{ 0x68, _T("Num 8"), KIND_NUMKEY },
	{ 0x69, _T("Num 9"), KIND_NUMKEY },
	{ 0x60, _T("Num 0"), KIND_NUMKEY },
	{ 0x14, _T("CapLock"), KIND_OTHER },
	{ VK_KANA, _T("Kana"), KIND_OTHER },
	{ 0x1C, _T("Convert"), KIND_OTHER },
	{ 0x1D, _T("NonConvert"), KIND_OTHER },
	{ 0x90, _T("NumLock"), KIND_OTHER },
	{ 0x70, _T("F1"), KIND_FUNCTION },
	{ 0x71, _T("F2"), KIND_FUNCTION },
	{ 0x72, _T("F3"), KIND_FUNCTION },
	{ 0x73, _T("F4"), KIND_FUNCTION },
	{ 0x74, _T("F5"), KIND_FUNCTION },
	{ 0x75, _T("F6"), KIND_FUNCTION },
	{ 0x76, _T("F7"), KIND_FUNCTION },
	{ 0x77, _T("F8"), KIND_FUNCTION },
	{ 0x78, _T("F9"), KIND_FUNCTION },
	{ 0x79, _T("F10"), KIND_FUNCTION },
	{ 0x7A, _T("F11"), KIND_FUNCTION },
	{ 0x7B, _T("F12"), KIND_FUNCTION },
	{ 0x7C, _T("F13"), KIND_FUNCTION },
	{ 0x7D, _T("F14"), KIND_FUNCTION },
	{ 0x7E, _T("F15"), KIND_FUNCTION },
	{ 0x7F, _T("F16"), KIND_FUNCTION },
	{ 0x80, _T("F17"), KIND_FUNCTION },
	{ 0x81, _T("F18"), KIND_FUNCTION },
	{ 0x82, _T("F19"), KIND_FUNCTION },
	{ 0x83, _T("F20"), KIND_FUNCTION },
	{ 0x84, _T("F21"), KIND_FUNCTION },
	{ 0x85, _T("F22"), KIND_FUNCTION },
	{ 0x86, _T("F23"), KIND_FUNCTION },
	{ 0x87, _T("F24"), KIND_FUNCTION },
};

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

	for (int i = 0; i < sizeof(VK_DEFINED_DATA) / sizeof(VK_DEFINED_DATA[0]); ++i) {
		if (VK_DEFINED_DATA[i].mVKCode == hotkey) {
			mVirtualKeyIdx = i;
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
	return 0 <= mVirtualKeyIdx && mVirtualKeyIdx < sizeof(VK_DEFINED_DATA) / sizeof(VK_DEFINED_DATA[0]);
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
	accel.key = VK_DEFINED_DATA[mVirtualKeyIdx].mVKCode;

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
	if (RegisterHotKey(targetWnd, ID_SOYOKAZE_TRY_HOTKEY, GetModifiers(), GetVKCode()) == FALSE) {
		return false;
	}
	UnregisterHotKey(targetWnd, ID_SOYOKAZE_TRY_HOTKEY);
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
	str += soyokaze::core::Honyaku::Get()->Literal(VK_DEFINED_DATA[mVirtualKeyIdx].mChar);
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

	return VK_DEFINED_DATA[mVirtualKeyIdx].mVKCode;
}

// Num0～Num9キーかどうか
bool HOTKEY_ATTR::IsNumKey() const
{
	return VK_DEFINED_DATA[mVirtualKeyIdx].mKind == KIND_NUMKEY;
}

// Functionキーかどうか
bool HOTKEY_ATTR::IsFunctionKey() const
{
	return VK_DEFINED_DATA[mVirtualKeyIdx].mKind == KIND_FUNCTION;
}

// 英字キー(A-Z)かどうか
bool HOTKEY_ATTR::IsAlphabetKey() const
{
	return VK_DEFINED_DATA[mVirtualKeyIdx].mKind == KIND_ALPHA;
}

// 何にも割り当てられていないか
bool HOTKEY_ATTR::IsUnmapped() const
{
	return mUseShift == FALSE && mUseCtrl == FALSE && mUseAlt == FALSE && mUseWin == FALSE && 
	       mVirtualKeyIdx == -1;
}
