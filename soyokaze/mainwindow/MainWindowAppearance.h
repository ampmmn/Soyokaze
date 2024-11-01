#pragma once

#include "mainwindow/WindowAppearanceIF.h"
#include "mainwindow/LauncherMainWindowIF.h"
#include "setting/AppPreferenceListenerIF.h"

namespace launcherapp {
namespace mainwindow {

class MainWindowAppearance : public WindowAppearnce, public AppPreferenceListenerIF
{
public:
	MainWindowAppearance(LauncherMainWindowIF* mainWnd);
	virtual ~MainWindowAppearance();

public:
// WindowAppearanceIF
	void OnShowWindow(BOOL bShow, UINT nStatus) override;
	void OnActivate(UINT nState, CWnd* wnd, BOOL bMinimized) override;
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor, HBRUSH defBr) override;
	void SetBlockDeactivateOnUnfocus(bool isBlock) override;

// AppPreferenceListenerIF
	void OnAppFirstBoot() override;
	void OnAppNormalBoot() override;
	void OnAppPreferenceUpdated() override;
	void OnAppExit() override;
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}


