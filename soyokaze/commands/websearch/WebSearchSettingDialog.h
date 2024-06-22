#pragma once

#include "gui/SinglePageDialog.h"
#include <memory>
#include "commands/websearch/WebSearchCommandParam.h"
#include "hotkey/HotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace websearch {

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

	void SetIcon(HICON icon);

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
	afx_msg LRESULT OnUserMessageIconChanged(WPARAM wp, LPARAM lp);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
