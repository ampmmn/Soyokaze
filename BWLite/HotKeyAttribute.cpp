#include "pch.h"
#include "framework.h"
#include "HotKeyAttribute.h"
#include "AppPreference.h"
#include <utility>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int ID_BWLITE_TRY_HOTKEY = 0xB31D;

static const std::pair<UINT, CString> VK_DEFINED_DATA[] = {
	{ 0x41, _T("A") },
	{ 0x42, _T("B") },
	{ 0x43, _T("C") },
	{ 0x44, _T("D") },
	{ 0x45, _T("E") },
	{ 0x46, _T("F") },
	{ 0x47, _T("G") },
	{ 0x48, _T("H") },
	{ 0x49, _T("I") },
	{ 0x4A, _T("J") },
	{ 0x4B, _T("K") },
	{ 0x4C, _T("L") },
	{ 0x4D, _T("M") },
	{ 0x4E, _T("N") },
	{ 0x4F, _T("O") },
	{ 0x50, _T("P") },
	{ 0x51, _T("Q") },
	{ 0x52, _T("R") },
	{ 0x53, _T("S") },
	{ 0x54, _T("T") },
	{ 0x55, _T("U") },
	{ 0x56, _T("V") },
	{ 0x57, _T("W") },
	{ 0x58, _T("X") },
	{ 0x59, _T("Y") },
	{ 0x5A, _T("Z") },
	{ 0x31, _T("1") },
	{ 0x32, _T("2") },
	{ 0x33, _T("3") },
	{ 0x34, _T("4") },
	{ 0x35, _T("5") },
	{ 0x36, _T("6") },
	{ 0x37, _T("7") },
	{ 0x38, _T("8") },
	{ 0x39, _T("9") },
	{ 0x30, _T("0") },
	{ 0x20, _T("Space") },
	{ 0x0D, _T("Enter") },
	{ 0x2E, _T("Delete") },
	{ 0x09, _T("Tab") },
	{ 0x26, _T("↑") },
	{ 0x28, _T("↓") },
	{ 0x25, _T("←") },
	{ 0x27, _T("→") },
	{ 0x6C, _T(",") },
	{ 0x6E, _T(".") },
	{ 0x6F, _T("/") },
	{ 0xBA, _T(":") },
	{ 0xBB, _T(";") },
	{ 0xC0, _T("@") },
	{ 0xDB, _T("[") },
	{ 0xDD, _T("]") },
	{ 0xBD, _T("^") },
	{ 0x6D, _T("-") },
	{ 0x22, _T("PageDown") },
	{ 0x21, _T("PageUp") },
	{ 0x24, _T("Home") },
	{ 0x23, _T("End") },
	{ 0x2D, _T("Insert") },
	{ 0x61, _T("Num 1") },
	{ 0x62, _T("Num 2") },
	{ 0x63, _T("Num 3") },
	{ 0x64, _T("Num 4") },
	{ 0x65, _T("Num 5") },
	{ 0x66, _T("Num 6") },
	{ 0x67, _T("Num 7") },
	{ 0x68, _T("Num 8") },
	{ 0x69, _T("Num 9") },
	{ 0x60, _T("Num 0") },
	{ 0x14, _T("CapLock") },
	{ VK_KANA, _T("かな") },
	{ 0x1C, _T("変換") },
	{ 0x1D, _T("無変換") },
	{ 0x90, _T("NumLock") },
	{ 0x70, _T("F1") },
	{ 0x71, _T("F2") },
	{ 0x72, _T("F3") },
	{ 0x73, _T("F4") },
	{ 0x74, _T("F5") },
	{ 0x75, _T("F6") },
	{ 0x76, _T("F7") },
	{ 0x77, _T("F8") },
	{ 0x78, _T("F9") },
	{ 0x79, _T("F10") },
	{ 0x7A, _T("F11") },
	{ 0x7B, _T("F12") },
	{ 0x7C, _T("F13") },
	{ 0x7D, _T("F14") },
	{ 0x7E, _T("F15") },
	{ 0x7F, _T("F16") },
	{ 0x80, _T("F17") },
	{ 0x81, _T("F18") },
	{ 0x82, _T("F19") },
	{ 0x83, _T("F20") },
	{ 0x84, _T("F21") },
	{ 0x85, _T("F22") },
	{ 0x86, _T("F23") },
	{ 0x87, _T("F24") },
};

// 修飾キー
// MOD_ALT     (0x0001)
// MOD_CONTROL (0x0002)
// MOD_SHIFT   (0x0004)
// MOD_WIN     (0x0008)
// MOD_NOREPEAT(0x4000)

HOTKEY_ATTR::HOTKEY_ATTR() : 
	mVirtualKeyIdx(0), 
	mUseShift(false), mUseCtrl(false), mUseAlt(false), mUseWin(false)
{
}

HOTKEY_ATTR::HOTKEY_ATTR(const HOTKEY_ATTR& rhs) :
	mVirtualKeyIdx(rhs.mVirtualKeyIdx), 
	mUseShift(rhs.mUseShift), mUseCtrl(rhs.mUseCtrl), mUseAlt(rhs.mUseAlt), mUseWin(rhs.mUseWin)
{
}


HOTKEY_ATTR::HOTKEY_ATTR(UINT modifiers, UINT hotkey) : 
	mVirtualKeyIdx(0)
{

	for (int i = 0; i < sizeof(VK_DEFINED_DATA) / sizeof(VK_DEFINED_DATA[0]); ++i) {
		if (VK_DEFINED_DATA[i].first == hotkey) {
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


HOTKEY_ATTR& HOTKEY_ATTR::operator = (const HOTKEY_ATTR& rhs)
{
	mUseShift = rhs.mUseShift;
	mUseCtrl = rhs.mUseCtrl;
	mUseAlt = rhs.mUseAlt;
	mUseWin = rhs.mUseWin;
	mVirtualKeyIdx = rhs.mVirtualKeyIdx;

	return *this;
}


/**
 *  ホットキーが登録可能かどうか調べる
 */
bool HOTKEY_ATTR::TryRegister(HWND targetWnd) const
{
	// 現在BWLiteが使用中のキーの場合はtrueを返す
	auto pref = AppPreference::Get();
	if (pref->mHotKeyVK == GetVKCode() && pref->mModifiers == GetModifiers()) {
		return true;
	}

	// 登録できるか実際に試す(すぐに解除)
	if (RegisterHotKey(targetWnd, ID_BWLITE_TRY_HOTKEY, GetVKCode(), GetModifiers()) == FALSE) {
		return false;
	}
	UnregisterHotKey(targetWnd, ID_BWLITE_TRY_HOTKEY);
	return true;
}

CString HOTKEY_ATTR::ToString() const
{
	if (mVirtualKeyIdx >= sizeof(VK_DEFINED_DATA) / sizeof(VK_DEFINED_DATA[0])) {
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
	str += VK_DEFINED_DATA[mVirtualKeyIdx].second;
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
	if (mVirtualKeyIdx >= sizeof(VK_DEFINED_DATA) / sizeof(VK_DEFINED_DATA[0])) {
		return 0;
	}

	return VK_DEFINED_DATA[mVirtualKeyIdx].first;
}

