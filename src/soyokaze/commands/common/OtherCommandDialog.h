#pragma once

#include "control/SinglePageDialog.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace common {

class OtherCommandDialog : public launcherapp::control::SinglePageDialog
{
public:
	struct Param {
		CString mCommandName;
		CString mCommandParam;
	};

public:
	OtherCommandDialog(LPCSTR helpId, CWnd* parentWnd = nullptr);
	virtual ~OtherCommandDialog();

	void SetParam(const Param& param);
	const Param& GetParam();

	void SetVariableDescription(LPCTSTR text);

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	// メッセージ欄
	CString mMessage;

public:
	Param mParam;
	CString mVariableText;
	int mCommandSelIndex;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp

