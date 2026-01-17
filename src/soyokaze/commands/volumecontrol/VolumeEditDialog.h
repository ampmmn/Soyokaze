#pragma once

#include "control/SinglePageDialog.h"
#include <memory>
#include "commands/volumecontrol/VolumeCommandParam.h"
#include "hotkey/CommandHotKeyDialog.h"

namespace launcherapp {
namespace commands {
namespace volumecontrol {

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

	void UpdateStatus();
// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnUpdateStatus();
	afx_msg void OnButtonHotKey();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
