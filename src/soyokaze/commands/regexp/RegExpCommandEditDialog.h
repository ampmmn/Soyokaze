#pragma once

#include "commands/regexp/RegExpCommandParam.h"
#include "control/SinglePageDialog.h"

#include <memory>

class IconLabel;

namespace launcherapp {
namespace commands {
namespace regexp {


class CommandEditDialog : public launcherapp::control::SinglePageDialog
{
public:
	CommandEditDialog(CWnd* parentWnd = nullptr);
	virtual ~CommandEditDialog();

	void SetName(const CString& name);
	void SetOriginalName(const CString& name);
	void SetParam(const CommandParam& param);
	const CommandParam& GetParam();

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	void ResolveShortcut(CString& path);

	struct PImpl;
	std::unique_ptr<PImpl> in;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonBrowseFile1Clicked();
	afx_msg void OnButtonBrowseDir1Clicked();
	afx_msg void OnButtonBrowseDir3Clicked();
	afx_msg void OnUpdateStatus();
	afx_msg void OnUpdatePath();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonResolveShortcut();
	afx_msg LRESULT OnUserMessageIconChanged(WPARAM wp, LPARAM lp);
	afx_msg void OnNotifyLinkOpen(NMHDR *pNMHDR, LRESULT *pResult);
};


}
}
}

