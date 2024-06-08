#pragma once

#include "gui/SinglePageDialog.h"
#include <memory>
#include "commands/ejectvolume/EjectVolumeCommandParam.h"
#include "hotkey/CommandHotKeyDialog.h"

namespace launcherapp {
namespace commands {
namespace ejectvolume {

// 
class SettingDialog : public launcherapp::gui::SinglePageDialog
{
protected:
	using Param = CommandParam; 

public:
	SettingDialog();
	virtual ~SettingDialog();

	void SetParam(const Param& param);
	const Param& GetParam();

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
