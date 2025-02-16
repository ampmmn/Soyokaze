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
	void SetIconPath(const CString& appIconPath);
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
	afx_msg void OnUpdateStatus();
	afx_msg void OnNotifyLinkOpen(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnButtonBrowse();
	afx_msg void OnButtonResetIcon();
	afx_msg void OnButtonResetFont();
	afx_msg void OnCbnKillfocusFontSize();
};

