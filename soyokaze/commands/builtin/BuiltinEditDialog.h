#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace builtin {

// 
class BuiltinEditDialog : public CDialogEx
{
public:
	BuiltinEditDialog(const CString& name, bool canEditEnable, bool canEditConfirm);
	virtual ~BuiltinEditDialog();

	void SetEnable(bool isEnable);
	bool GetEnable();
	void SetConfirm(bool isConfirm);
	bool GetConfirm();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();
// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
