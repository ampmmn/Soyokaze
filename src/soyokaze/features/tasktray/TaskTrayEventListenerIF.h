#pragma once


class TaskTrayEventListenerIF
{
public:
	virtual ~TaskTrayEventListenerIF() {}

	virtual LRESULT OnTaskTrayLButtonDblclk() = 0;
	virtual LRESULT OnTaskTrayContextMenu(CWnd* wnd, CPoint point) = 0;


};
