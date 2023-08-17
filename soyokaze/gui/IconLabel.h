#pragma once

class IconLabel : public CStatic
{
public:
	IconLabel();
	virtual ~IconLabel();

	void DrawIcon(HICON iconHandle);
	void DrawDefaultIcon();

private:
	void DrawIcon(CDC* pDC, HICON iconHandle);

protected:
	HICON mIconDefault;
	CBitmap mBuffer;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
};

