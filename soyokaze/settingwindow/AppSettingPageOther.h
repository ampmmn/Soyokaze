#pragma once

#include <memory>
#include "gui/SettingPage.h"

// 
class AppSettingPageOther : public SettingPage
{
public:
	AppSettingPageOther(CWnd* parentWnd);
	virtual ~AppSettingPageOther();

	struct PImpl;
	std::unique_ptr<PImpl> in;

protected:
	bool UpdateStatus();

	BOOL OnKillActive() override;
	BOOL OnSetActive() override;
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	void OnEnterSettings() override;
	bool GetHelpPageId(CString& id) override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnTransparencyChanged();
	afx_msg void OnCheckWarnLongTime();
};

