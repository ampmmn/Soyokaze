#pragma once

#include <memory>

class MouseoverActivateWindow : public CWnd
{
public:
	MouseoverActivateWindow();
	~MouseoverActivateWindow();

	BOOL Create(CWnd* parentWnd);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;

	afx_msg void OnTimer(UINT_PTR timerId);

	DECLARE_MESSAGE_MAP()
};
