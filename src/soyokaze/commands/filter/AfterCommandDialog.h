#pragma once

#include "gui/SinglePageDialog.h"
#include <memory>
#include "commands/filter/FilterCommandParam.h"

namespace launcherapp {
namespace commands {
namespace filter {

class AfterCommandDialog : public launcherapp::gui::SinglePageDialog
{
public:
	AfterCommandDialog(CWnd* parentWnd = nullptr);
	virtual ~AfterCommandDialog();

	void SetParam(const CommandParam& param);
	const CommandParam& GetParam();

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	// メッセージ欄
	CString mMessage;

public:
	CommandParam mParam;
	int mCommandSelIndex;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

