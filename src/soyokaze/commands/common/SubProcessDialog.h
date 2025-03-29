#pragma once

#include "gui/SinglePageDialog.h"
#include <memory>
#include "commands/filter/FilterCommandParam.h"
#include "hotkey/CommandHotKeyAttribute.h"

class IconLabel;

namespace launcherapp {
namespace commands {
namespace common {


class SubProcessDialog : public launcherapp::gui::SinglePageDialog
{
public:
	struct Param {
		CString mFilePath;
		CString mCommandParam;
		CString mWorkDir;
		int mShowType = SW_SHOW;
	};

public:
	SubProcessDialog(LPCTSTR helpId, CWnd* parentWnd = nullptr);
	virtual ~SubProcessDialog();

	void SetParam(const Param& param);
	const Param& GetParam();

	void SetVariableDescription(LPCTSTR text);

	bool UpdateStatus();
	void OpenTarget();

	static bool IsEditableFileType(CString path);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

public:
	Param mParam;
	CString mVariableText;

	CMFCMenuButton mPathMenuBtn;
	CMenu mMenuForPathBtn;

	CString mMessage;
// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg void OnSelectFilePath();
	afx_msg void OnSelectFolderPath();
	afx_msg void OnPathMenuBtnClicked();
	afx_msg void OnSelectWorkDir();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

