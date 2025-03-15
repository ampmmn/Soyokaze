#pragma once

#include "gui/SettingPage.h"
#include <memory>

// 
class ExecSettingDialog : public SettingPage
{
public:
	ExecSettingDialog(CWnd* parentWnd);
	virtual ~ExecSettingDialog();

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
	afx_msg void OnButtonBrowseFile();
	afx_msg void OnCheckUseFilter();
};

