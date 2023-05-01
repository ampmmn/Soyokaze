#pragma once

class CBWLiteDlg;

// タスクトレイに表示するウインドウ
class TaskTray : public CWnd
{
public:
	TaskTray(CBWLiteDlg* window);
	virtual ~TaskTray();

	DECLARE_DYNAMIC(TaskTray)

protected:
	HICON mIcon;
	HWND mTaskTrayWindow;
	CBWLiteDlg* mBWLiteWindowPtr;

public:
	BOOL Create();

	LRESULT OnNotifyTrakTray(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
	void OnContextMenu();
};

