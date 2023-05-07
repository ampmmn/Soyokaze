#pragma once

class CBWLiteDlg;

// 繧ｿ繧ｹ繧ｯ繝医Ξ繧､縺ｫ陦ｨ遉ｺ縺吶ｋ繧ｦ繧､繝ｳ繝峨え
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

