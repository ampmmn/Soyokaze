#pragma once

#include <memory>
#include "gui/SettingPage.h"

// 
class ViewSettingDialog : public SettingPage
{
public:
	ViewSettingDialog(CWnd* parentWnd);
	virtual ~ViewSettingDialog();

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
};

