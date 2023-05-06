#pragma once

class HOTKEY_ATTR
{
public:
	HOTKEY_ATTR();
	HOTKEY_ATTR(const HOTKEY_ATTR& rhs);
	HOTKEY_ATTR(UINT modifiers, UINT hotkey);

	HOTKEY_ATTR& operator = (const HOTKEY_ATTR& rhs);

	bool TryRegister(HWND targetWnd) const;

	CString ToString() const;

	UINT GetModifiers() const;
	UINT GetVKCode() const;

	BOOL mUseShift;
	BOOL mUseCtrl;
	BOOL mUseAlt;
	BOOL mUseWin;
	int mVirtualKeyIdx;
};

