#pragma once

namespace launcherapp {
namespace mainwindow {

class WindowAppearnce
{
public:
	virtual ~WindowAppearnce() {}

	virtual void OnShowWindow(BOOL bShow, UINT nStatus) = 0;
	virtual void OnActivate(UINT nState, CWnd* wnd, BOOL bMinimized) = 0;
	virtual HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor, HBRUSH defBr) = 0;
	virtual void SetBlockDeactivateOnUnfocus(bool isBlock) = 0;

};

}
}


