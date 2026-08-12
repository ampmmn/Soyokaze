#pragma once

class LauncherShutdownWindow : public CWnd
{
public:
	BOOL Create();

protected:
	afx_msg BOOL OnQueryEndSession();

	DECLARE_MESSAGE_MAP()
};
