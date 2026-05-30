#pragma once

class ImageLabel : public CStatic
{
public:
	ImageLabel();
	virtual ~ImageLabel();

	CBitmap* GetBitmap();

protected:
	CBitmap mBuffer;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
};

