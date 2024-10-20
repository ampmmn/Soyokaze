// あ
#pragma once

// ホットキーの属性
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

	bool IsReservedKey() const;
	bool IsUnmapped() const;

	CString ToString() const;

	UINT GetModifiers() const;
	UINT GetVKCode() const;

	void Reset();

	bool mUseShift;
	bool mUseCtrl;
	bool mUseAlt;
	bool mUseWin;
	short mVirtualKeyIdx;

	static const VK_ITEM VK_DEFINED_DATA[];

};

