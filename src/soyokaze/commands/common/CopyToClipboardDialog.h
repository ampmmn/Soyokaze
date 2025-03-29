#pragma once

#include "gui/SinglePageDialog.h"
#include <memory>
#include "commands/filter/FilterCommandParam.h"

namespace launcherapp {
namespace commands {
namespace common {

class CopyToClipboardDialog : public launcherapp::gui::SinglePageDialog
{
public:
	class Param {
	public:
		CString mCommandParam;
	};

public:
	CopyToClipboardDialog(LPCTSTR helpId, CWnd* parentWnd = nullptr);
	virtual ~CopyToClipboardDialog();

	void SetParam(const Param& param);
	const Param& GetParam();

	void SetVariableDescription(LPCTSTR text);

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

public:
	Param mParam;
	CString mVariableText;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
};

} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp

