#pragma once

#include "gui/SinglePageDialog.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace builtin {

// 
class BuiltinEditDialog : public launcherapp::gui::SinglePageDialog
{
public:
	BuiltinEditDialog(const CString& name, const CString& description, bool canEditEnable, bool canEditConfirm);
	virtual ~BuiltinEditDialog();

	CString GetName();

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
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
