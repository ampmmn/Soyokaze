#pragma once

#include "control/SinglePageDialog.h"
#include <memory>
#include "commands/activate_window/WindowActivateCommandParam.h"
#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace activate_window {

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

	void SetParam(const Param& param);
	const Param& GetParam();

	void ResetHotKey();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

// 実装
protected:
	bool UpdateStatus();

	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonHotKey();
	afx_msg void OnButtonTest();
	afx_msg void OnUpdateStatus();
	LRESULT OnUserMessageCaptureWindow(WPARAM wParam, LPARAM lParam);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
