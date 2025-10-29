#pragma once

#include <memory>

namespace launcherapp { namespace core {
class Command;
}}


namespace launcherapp { namespace mainwindow {
class LauncherMainWindowIF;
}}

namespace launcherapp { namespace mainwindow { namespace guide {


class GuideCtrl : public CWnd
{
public:
	GuideCtrl();
	virtual ~GuideCtrl();

public:
	static bool Initialize();

	void SetMainWindow(LauncherMainWindowIF* mainWnd);
	void SetClickNotifyMessageId(UINT msgId);

	bool Draw(launcherapp::core::Command* cmd);

private:
	DECLARE_MESSAGE_MAP()
	static LRESULT CALLBACK WindowProc(HWND h, UINT msg, WPARAM wp, LPARAM lp);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR timerId);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnLButtonDown(UINT flags, CPoint pt);
	afx_msg void OnMouseMove(UINT flags, CPoint pt);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}}
