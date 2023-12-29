#pragma once

#include "gui/SettingPage.h"

// 
class AppSettingPathPage : public SettingPage
{
public:
	AppSettingPathPage(CWnd* parentWnd);
	virtual ~AppSettingPathPage();

	struct PImpl;
	std::unique_ptr<PImpl> in;

protected:
	void SwapItem(int srcIndex, int dstIndex);
	bool UpdateStatus();

	BOOL OnKillActive() override;
	BOOL OnSetActive() override;
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	void OnEnterSettings() override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonUp();
	afx_msg void OnButtonDown();
	afx_msg void OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyItemDblClk(NMHDR *pNMHDR, LRESULT *pResult);
};

