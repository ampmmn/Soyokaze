#pragma once

#include "gui/SinglePageDialog.h"
#include "commands/remote/RemoteClientCommandParam.h"

#include <memory>

namespace launcherapp { namespace commands { namespace remote {

class CommandEditDialog : public launcherapp::gui::SinglePageDialog
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

	// 編集開始時のコマンド名
	CString mOrgName;

	// メッセージ欄
	CString mMessage;
	// ホットキー(表示用)
	CString mHotKey;

public:
	CommandParam mParam;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonHotKey();
	afx_msg void OnNotifyLinkOpen(NMHDR *pNMHDR, LRESULT *pResult);
};

}}} // end of namespace launcherapp::commands::remote
