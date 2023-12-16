#pragma once

class CSoyokazeDlg;

class TaskTray : public CWnd
{
public:
	TaskTray(CSoyokazeDlg* window);
	virtual ~TaskTray();

	DECLARE_DYNAMIC(TaskTray)

	void ShowMessage(const CString& msg);
	void ShowMessage(const CString& msg, const CString& title);

protected:
	HICON mIcon;
	HWND mTaskTrayWindow;
	CSoyokazeDlg* mSoyokazeWindowPtr;

	NOTIFYICONDATA mNotifyIconData;

public:
	BOOL Create();

	LRESULT OnNotifyTrakTray(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
	void OnContextMenu();
};

