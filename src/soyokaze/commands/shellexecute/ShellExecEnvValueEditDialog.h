#pragma once

#include "gui/SinglePageDialog.h"

namespace launcherapp { namespace commands { namespace shellexecute {

// 環境変数を追加・編集する画面
class ValueEditDialog : public launcherapp::gui::SinglePageDialog
{
public:
	ValueEditDialog(CWnd* parentWnd = nullptr);
	~ValueEditDialog();

public:
	CString GetName();
	void SetName(const CString& name);
	CString GetValue();
	void SetValue(const CString& value);


	bool ValidateNameAndValue(const CString& name, const CString& value);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

public:
	CString mName;
	CString mValue;
	CString mMessage;
	bool mCanEditName = true;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnUpdateStatus();
	afx_msg void OnButtonBrowseFile();
	afx_msg void OnButtonBrowseDir();
};

}}} // end of namespace launcherapp::commands::shellexecute
