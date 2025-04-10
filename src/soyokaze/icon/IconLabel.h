#pragma once

#include <memory>

class IconLabel : public CStatic
{
public:
	IconLabel();
	virtual ~IconLabel();

	void EnableIconChange();

	void DrawIcon(HICON iconHandle);
	void DrawDefaultIcon();

	void SetBackgroundColor(bool isUseSystemSetting, COLORREF cr);

private:
	void DrawIcon(CDC* pDC, HICON iconHandle);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnMenuChangeIcon();
	afx_msg void OnMenuDefaultIcon();
};

