#pragma once

#include "gui/SinglePageDialog.h"
#include <memory>
#include "commands/filter/FilterCommandParam.h"

namespace launcherapp {
namespace commands {
namespace filter {

class AfterCopyDialog : public launcherapp::gui::SinglePageDialog
{
public:
	AfterCopyDialog(CWnd* parentWnd = nullptr);
	virtual ~AfterCopyDialog();

	void SetParam(const CommandParam& param);
	const CommandParam& GetParam();

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

public:
	CommandParam mParam;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

