#pragma once

#include "tasktray/TaskTrayEventListenerIF.h"
#include <memory>

class TaskTray : public CWnd
{
public:
	TaskTray(TaskTrayEventListenerIF* listener);
	virtual ~TaskTray();

	DECLARE_DYNAMIC(TaskTray)

	void ShowMessage(const CString& msg);
	void ShowMessage(const CString& msg, const CString& title);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;

public:
	BOOL Create();

	LRESULT OnNotifyTrakTray(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
	void OnContextMenu();
};

