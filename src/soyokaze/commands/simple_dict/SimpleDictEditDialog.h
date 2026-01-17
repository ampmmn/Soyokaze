#pragma once

#include "control/SinglePageDialog.h"
#include <memory>
#include "commands/simple_dict/SimpleDictParam.h"
#include "hotkey/CommandHotKeyAttribute.h"

class ModalComboBox;

namespace launcherapp {
namespace commands {
namespace simple_dict {

class SettingDialog : public launcherapp::control::SinglePageDialog
{
public:
	SettingDialog(CWnd* parentWnd = nullptr);
	virtual ~SettingDialog();

	void SetName(const CString& name);
	void SetOriginalName(const CString& name);
	void SetParam(const SimpleDictParam& param);
	const SimpleDictParam& GetParam() const;

	void ResetHotKey();

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	struct PImpl;
	std::unique_ptr<PImpl> in;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateName();
	afx_msg void OnUpdateCondition();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnUpdateStatus();
	afx_msg void OnButtonFilePath();
	afx_msg void OnButtonTest();
	afx_msg void OnButtonFrontRange();
	afx_msg void OnButtonBackRange();
	afx_msg void OnButtonValue2Range();
	afx_msg void OnButtonHotKey();
	afx_msg void OnNotifyLinkOpenMacro(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNotifyLinkOpenTebiki(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTypeMenuBtnClicked();
	afx_msg void OnSelectExecOtherCommand();
	afx_msg void OnSelectSubProcess();
	afx_msg void OnSelectCopyClipboard();
};

} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

