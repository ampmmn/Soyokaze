#pragma once

#include "settingwindow/AppSettingPageBase.h"
#include "settingwindow/AppSettingPageRepository.h"
#include <memory>

class AppSettingPageCommandPriority :
 	virtual public launcherapp::settingwindow::AppSettingPageBase
{
public:
	AppSettingPageCommandPriority();
	~AppSettingPageCommandPriority();

	// $B%&%$%s%I%&$r:n@.$9$k(B
	bool Create(HWND parentWindow) override;
	// $B%&%$%s%I%&%O%s%I%k$r<hF@$9$k(B
	HWND GetHwnd() override;
	// $BF1$8?F$NCf$GI=<($9$k=g=x(B($BDc$$$[$I@h$KI=<((B)
	int GetOrder() override;
	// 
	bool OnEnterSettings() override;
	// $B%Z!<%8$,%"%/%F%#%V$K$J$k$H$-$K8F$P$l$k(B
	bool OnSetActive() override;
	// $B%Z!<%8$,Hs%"%/%F%#%V$K$J$k$H$-$K8F$P$l$k(B
	bool OnKillActive() override;
	//
	void OnOKCall() override;

	// $B%Z!<%8$K4XO"IU$1$i$l$?%X%k%W%Z!<%8(BID$B$r<hF@$9$k(B
	bool GetHelpPageId(CString& helpPageId) override;

	// $B%$%s%9%?%s%9$rJ#@=$9$k(B
	AppSettingPageIF* Clone() override { return new AppSettingPageCommandPriority(); }

	DECLARE_APPSETTINGPAGE(AppSettingPageCommandPriority)
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


