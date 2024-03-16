#pragma once

#include "tasktray/TaskTrayEventListenerIF.h"

class TaskTray : public CWnd
{
public:
	TaskTray(TaskTrayEventListenerIF* listener);
	virtual ~TaskTray();

	DECLARE_DYNAMIC(TaskTray)

	void ShowMessage(const CString& msg);
	void ShowMessage(const CString& msg, const CString& title);

protected:
	// タスクトレイアイコン
	HICON mIcon;
	// タスクトレイウインドウ
	HWND mTaskTrayWindow;
	// タスクトレイ関連イベント通知先
	TaskTrayEventListenerIF* mListenerPtr;

	// タスクトレイに登録用の情報
	NOTIFYICONDATA mNotifyIconData;

public:
	BOOL Create();

	LRESULT OnNotifyTrakTray(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
	void OnContextMenu();
};

