#pragma once

class HOTKEY_ATTR
{
	struct VK_ITEM;
public:
	HOTKEY_ATTR();
	HOTKEY_ATTR(const HOTKEY_ATTR& rhs);
	HOTKEY_ATTR(UINT modifiers, UINT hotkey);

	bool operator == (const HOTKEY_ATTR& rhs) const;
	bool operator != (const HOTKEY_ATTR& rhs) const;
	bool operator < (const HOTKEY_ATTR& rhs) const;

	HOTKEY_ATTR& operator = (const HOTKEY_ATTR& rhs);

	bool IsValid() const;

	bool GetAccel(ACCEL& accel) const;

	bool TryRegister(HWND targetWnd) const;

	bool IsNumKey() const;
	bool IsFunctionKey() const;
	bool IsAlphabetKey() const;

	bool IsUnmapped() const;

	CString ToString() const;

	UINT GetModifiers() const;
	UINT GetVKCode() const;

	BOOL mUseShift;
	BOOL mUseCtrl;
	BOOL mUseAlt;
	BOOL mUseWin;
	int mVirtualKeyIdx;

	static const VK_ITEM VK_DEFINED_DATA[];

};

