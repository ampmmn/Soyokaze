#pragma once

class IconLabelForApp : public CStatic
{
public:
	IconLabelForApp();
	virtual ~IconLabelForApp();

	void EnableIconChange();
	void DisableIconChange();

	void DrawIcon(HICON iconHandle);
	void DrawDefaultIcon();

	void SetBackgroundColor(bool isUseSystemSetting, COLORREF cr);

private:
	void DrawIcon(CDC* pDC, HICON iconHandle);

protected:
	HICON mIconDefault;
	CBitmap mBuffer;
	bool mCanIconChange;
	bool mIsUseBackgroundColor;
	COLORREF mBackgroundColor;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnMenuChangeIcon();
	afx_msg void OnMenuDefaultIcon();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);


};

