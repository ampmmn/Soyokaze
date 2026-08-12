#pragma once

class LauncherSystemEventWindow : public CWnd
{
public:
	LauncherSystemEventWindow();
	virtual ~LauncherSystemEventWindow();

	BOOL Create();

protected:
	afx_msg void OnTimer(UINT_PTR timerId);
	afx_msg LRESULT OnMessageSessionChange(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
