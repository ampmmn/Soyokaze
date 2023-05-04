#pragma once

class IconLabel : public CStatic
{
public:
	IconLabel();
	virtual ~IconLabel();

	void DrawIcon(HICON iconHandle);

protected:
	CDC mMemDC;
	CBitmap mMemBmp;
	CBrush mBkBrush;
	HICON mIconDefault;

// ŽÀ‘•
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
};

