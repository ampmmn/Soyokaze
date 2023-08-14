#pragma once

#include <memory>

class SettingPage;


class SettingDialogBase : public CDialogEx
{
public:
	// コンストラクタ
	SettingDialogBase();
	// デストラクタ
	virtual ~SettingDialogBase();

protected:
	virtual HTREEITEM OnSetupPages() = 0;

	HTREEITEM AddPage(HTREEITEM hParent, std::unique_ptr<SettingPage>& page, void* param);

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
	afx_msg LRESULT OnUserEnableOKButton(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnUserDisableOKButton(WPARAM wp, LPARAM lp);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


