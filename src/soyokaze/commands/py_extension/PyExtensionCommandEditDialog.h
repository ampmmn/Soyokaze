#pragma once

#include "control/SinglePageDialog.h"
#include "commands/py_extension/PyExtensionCommandParam.h"

#include <memory>

class IconLabel;

namespace launcherapp { namespace commands { namespace py_extension {


class CommandEditDialog : public launcherapp::control::SinglePageDialog
{
public:
	CommandEditDialog(CWnd* parentWnd = nullptr);
	virtual ~CommandEditDialog();

	void SetName(const CString& name);
	void SetOriginalName(const CString& name);
	void SetParam(const CommandParam& param);
	const CommandParam& GetParam();

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* msg);

	void UpdateTitle();
	bool TestSyntax();
	BOOL UpdateDataWrapper(BOOL bSaveAndValidate = TRUE);

	// 編集開始時のコマンド名
	CString mOrgName;

	CString mArguments;

	// メッセージ欄
	CString mMessage;
	bool mIsTested{true};
	bool mIsError{false};

	//
	CString mResultMsg;
	// ホットキー(表示用)
	CString mHotKey;

public:
	CommandParam mParam;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg void OnScriptChanged();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonHotKey();
	afx_msg void OnButtonSyntaxCheck();
	afx_msg void OnButtonRun();
	afx_msg void OnNotifyLinkOpen(NMHDR *pNMHDR, LRESULT *pResult);
};


}}}

