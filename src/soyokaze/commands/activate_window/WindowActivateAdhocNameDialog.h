#pragma once

#include "control/SinglePageDialog.h"

namespace launcherapp {
namespace commands {
namespace activate_window {

class AdhocNameDialog : public launcherapp::control::SinglePageDialog
{
public:
	AdhocNameDialog(CWnd* parentWnd = nullptr);

	const CString& GetName();
	void SetName(const CString& name);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
protected:
		CString mName;
};

}
}
}

