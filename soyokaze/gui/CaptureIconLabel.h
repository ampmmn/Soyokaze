#pragma once

#include "IconLabel.h"

class CaptureIconLabel : public IconLabel
{
public:
	CaptureIconLabel();
	virtual ~CaptureIconLabel();

protected:
	HCURSOR mCursorHandle;
	HCURSOR mOldCursorHandle;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT, CPoint);
	afx_msg void OnLButtonUp(UINT, CPoint);
};

