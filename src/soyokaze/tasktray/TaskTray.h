#pragma once

#include "tasktray/TaskTrayEventListenerIF.h"
#include <memory>

class TaskTray : public CWnd
{
public:
	TaskTray(TaskTrayEventListenerIF* listener);
	virtual ~TaskTray();

	DECLARE_DYNAMIC(TaskTray)

	void ShowMessage(const wchar_t* msg, const wchar_t* title = L"");
	void ShowMessage(const char* msg, const char* title = "");

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;

public:
	BOOL Create();

	LRESULT OnNotifyTrakTray(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
	void OnContextMenu();
};

