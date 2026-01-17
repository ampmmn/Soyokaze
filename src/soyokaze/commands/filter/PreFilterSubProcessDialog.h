#pragma once

#include "control/SinglePageDialog.h"
#include <memory>
#include "commands/filter/FilterCommandParam.h"

namespace launcherapp {
namespace commands {
namespace filter {

class PreFilterSubProcessDialog : public launcherapp::control::SinglePageDialog
{
public:
	PreFilterSubProcessDialog(CWnd* parentWnd = nullptr);
	virtual ~PreFilterSubProcessDialog();

	void SetParam(const CommandParam& param);
	const CommandParam& GetParam();

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	// メッセージ欄
	CString mMessage;

public:
	CommandParam mParam;
	int mPreFilterCodePageIndex;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg void OnButtonBrowseFile1Clicked();
	afx_msg void OnButtonBrowseDir3Clicked();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnNotifyLinkOpen(NMHDR *pNMHDR, LRESULT *pResult);
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

