#pragma once

#include <memory>
#include "gui/SettingPage.h"

// 
class AppSettingPageCommandPriority : public SettingPage
{
public:
	AppSettingPageCommandPriority(CWnd* parentWnd);
	virtual ~AppSettingPageCommandPriority();

	struct PImpl;
	std::unique_ptr<PImpl> in;

protected:
	void UpdateListItems();
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
	afx_msg void OnEditFilterChanged();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonResetAll();
	afx_msg void OnLvnItemChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHeaderClicked(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFindCommand(NMHDR* pNMHDR, LRESULT* pResult);
};


