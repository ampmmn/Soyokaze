#pragma once

#include "gui/HotKeyDialog.h"
#include "Settings.h"

// 
class SettingDialog : public CDialogEx
{
public:
	SettingDialog();
	virtual ~SettingDialog();

	void SetSettings(const Settings& settings);
	const Settings& GetSettings();

protected:
	bool SelectPage(HTREEITEM hTreeCtrl);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();
	virtual void OnOK();

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnTvnSelChangingPage(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

protected:
	struct PImpl;
	PImpl* in;
};

