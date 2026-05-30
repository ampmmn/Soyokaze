#pragma once

#include "setting/Settings.h"
#include <memory>

// 
class AppSettingDialog : public CDialogEx
{
public:
	AppSettingDialog(CWnd* parentWnd = nullptr);
	virtual ~AppSettingDialog();

	CString GetBreadCrumbsString();
	void SetBreadCrumbsString(const CString& crumbs);

	void SetSettings(const Settings& settings);
	const Settings& GetSettings();

	bool SelectPage(HTREEITEM hTreeCtrl);
	bool ShowHelp();

	HTREEITEM OnSetupPages();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	BOOL PreTranslateMessage(MSG* pMsg) override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnTvnSelChangingPage(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnUserEnableOKButton(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnUserDisableOKButton(WPARAM wp, LPARAM lp);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint pt);
	afx_msg void OnCommandHelp();


protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

