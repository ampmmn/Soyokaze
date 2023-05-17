#pragma once

class CSoyokazeDlg;

class TaskTray : public CWnd
{
public:
	TaskTray(CSoyokazeDlg* window);
	virtual ~TaskTray();

	DECLARE_DYNAMIC(TaskTray)

protected:
	HICON mIcon;
	HWND mTaskTrayWindow;
	CSoyokazeDlg* mSoyokazeWindowPtr;

public:
	BOOL Create();

	LRESULT OnNotifyTrakTray(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
	void OnContextMenu();
};

