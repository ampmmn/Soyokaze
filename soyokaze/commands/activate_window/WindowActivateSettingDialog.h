#pragma once

#include <memory>
#include "commands/activate_window/WindowActivateCommandParam.h"
#include "HotKeyAttribute.h"

namespace soyokaze {
namespace commands {
namespace activate_window {

// 
class SettingDialog : public CDialogEx
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
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonHotKey();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
