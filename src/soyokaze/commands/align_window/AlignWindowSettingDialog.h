#pragma once

#include "control/SinglePageDialog.h"
#include <memory>
#include "commands/align_window/AlignWindowCommandParam.h"
#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace align_window {

// 
class SettingDialog : public launcherapp::control::SinglePageDialog
{
protected:
	using Param = CommandParam; 

public:
	SettingDialog(CWnd* parentWnd = nullptr);
	virtual ~SettingDialog();

	void SetName(const CString& name);
	void SetOriginalName(const CString& name);
	void ResetHotKey();

	void SetParam(const Param& param);
	const Param& GetParam();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

// 実装
protected:
	bool UpdateStatus();

	void SwapItem(int srcIndex, int dstIndex);
	void SetItemToList(int index, const Param::ITEM& item);

	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonHotKey();
	afx_msg void OnUpdateStatus();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonUp();
	afx_msg void OnButtonDown();
	afx_msg void OnNotifyItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyItemDblClk(NMHDR *pNMHDR, LRESULT *pResult);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
