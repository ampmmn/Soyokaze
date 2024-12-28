#pragma once

class VirtualKeyDefine
{
public:
	enum {
		KIND_ALPHA,
		KIND_NUMBER,
		KIND_CHAR,
		KIND_MOVE,
		KIND_NUMKEY,
		KIND_FUNCTION,
		KIND_OTHER,
	};

	struct ITEM
	{
		ITEM(UINT vk, LPCTSTR chr, int kind);
		ITEM(const ITEM&) = default;
		ITEM& operator = (const ITEM&) = default;

		UINT mVKCode;
		CString mChar;
		int mKind;
	};

	VirtualKeyDefine();
	~VirtualKeyDefine();

	int GetItemCount() const;
	ITEM GetItem(int index) const;

	static VirtualKeyDefine* GetInstance();
};

