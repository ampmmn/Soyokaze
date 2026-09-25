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
	/** モニター構成変更通知を遅延処理するタイマーを設定する */
	afx_msg LRESULT OnMessageMonitorConfigurationChange(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
