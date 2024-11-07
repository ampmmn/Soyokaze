// あ
#pragma once

// ホットキーの属性
class SANDSKEY_ATTR
{
public:
	enum ModifierFlag
	{
		MOD_SPACE,
		MOD_CAPSLOCK,
		MOD_CONVERT,
		MOD_NONCONVERT,
		MOD_KANA,
		MOD_UNINITIALIZED = -1,
	};
public:
	SANDSKEY_ATTR();
	SANDSKEY_ATTR(const SANDSKEY_ATTR& rhs);
	SANDSKEY_ATTR(UINT modifier, UINT hotkey);

	bool operator == (const SANDSKEY_ATTR& rhs) const;
	bool operator != (const SANDSKEY_ATTR& rhs) const;
	bool operator < (const SANDSKEY_ATTR& rhs) const;

	SANDSKEY_ATTR& operator = (const SANDSKEY_ATTR& rhs);

	bool IsValid() const;

	bool IsUnmapped() const;

	CString ToString() const;

	UINT GetModifier() const;
	UINT GetVKCode() const;
	UINT GetModifierVKCode() const;

	void Reset();

	short mModifier;
	short mVirtualKeyIdx;
};

