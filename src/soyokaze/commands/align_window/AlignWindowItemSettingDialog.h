#pragma once

#include "control/SinglePageDialog.h"
#include <memory>
#include "commands/align_window/AlignWindowCommandParam.h"
#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace align_window {

// 
class ItemDialog : public launcherapp::control::SinglePageDialog
{
protected:
	using Param = CommandParam::ITEM; 

public:
	ItemDialog(CWnd* parentWnd);
	virtual ~ItemDialog();

	void SetParam(const Param& param);
	const Param& GetParam();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

// 実装
protected:
	bool UpdateStatus();

	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonUpdate();
	afx_msg void OnUpdateStatus();
	LRESULT OnUserMessageCaptureWindow(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyItemClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyItemDblClk(NMHDR *pNMHDR, LRESULT *pResult);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
