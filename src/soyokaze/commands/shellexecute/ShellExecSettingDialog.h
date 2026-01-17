#pragma once

#include <memory>
#include "control/SettingDialogBase.h"
#include "commands/shellexecute/ShellExecCommandParam.h"

namespace launcherapp {
namespace commands {
namespace shellexecute {

// 
class SettingDialog : public SettingDialogBase
{
public:
	SettingDialog(CWnd* parentWnd = nullptr);
	virtual ~SettingDialog();

	void SetName(const CString& name);
	void SetOriginalName(const CString& name);
	void SetParam(const CommandParam& param);
	const CommandParam& GetParam();

	void ResetHotKey();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

// 実装
protected:
	DECLARE_MESSAGE_MAP()

	HTREEITEM OnSetupPages() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
