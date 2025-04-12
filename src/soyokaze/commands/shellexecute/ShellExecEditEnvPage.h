#pragma once

#include "gui/SettingPage.h"
#include "commands/shellexecute/ShellExecCommandParam.h"
#include <memory>

namespace launcherapp { namespace commands { namespace shellexecute {

class SettingPageEnv : public SettingPage
{
	using CommandParam = launcherapp::commands::shellexecute::CommandParam;

public:
	SettingPageEnv(CWnd* parentWnd);
	virtual ~SettingPageEnv();

	bool UpdateStatus();

	void InitListCtrl();
	bool EditItem(int index);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

// 実装
protected:
	BOOL OnKillActive() override;
	BOOL OnSetActive() override;
	void OnEnterSettings() override;
	bool GetHelpPageId(CString& id) override;

	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonDelete();
	afx_msg void OnUpdateStatus();
	afx_msg void OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyItemClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyItemDblClk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}}} // end of namespace launcherapp::commands::shellexecute

