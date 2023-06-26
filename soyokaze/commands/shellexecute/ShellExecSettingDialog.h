#pragma once

#include "gui/SettingDialogBase.h"
#include "commands/shellexecute/ShellExecCommandParam.h"

namespace soyokaze {
namespace commands {
namespace shellexecute {

// 
class SettingDialog : public SettingDialogBase
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

// 実装
protected:
	DECLARE_MESSAGE_MAP()

	HTREEITEM OnSetupPages() override;

protected:
	struct PImpl;
	PImpl* in;
};

}
}
}
