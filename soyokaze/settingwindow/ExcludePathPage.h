#pragma once

#include "gui/SettingPage.h"

// 
class ExcludePathPage : public SettingPage
{
public:
	ExcludePathPage(CWnd* parentWnd);
	virtual ~ExcludePathPage();

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
	bool GetHelpPageId(CString& id) override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonDelete();
	afx_msg void OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyItemDblClk(NMHDR *pNMHDR, LRESULT *pResult);
};

