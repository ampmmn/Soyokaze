#pragma once

#include "gui/SinglePageDialog.h"
#include <memory>

class IconLabel;

namespace launcherapp {
namespace commands {
namespace watchpath {

class CommandEditDialog : public launcherapp::gui::SinglePageDialog
{
public:
	CommandEditDialog();
	virtual ~CommandEditDialog();

	void SetOrgName(const CString& name);
	void SetName(const CString& name);
	void SetDescription(const CString& desc);
	void SetNotifyMessage(const CString& msg);

	int GetShowType();
	void SetShowType(int type);

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	void ResolveShortcut(CString& path);

	// 編集開始時のコマンド名
	CString mOrgName;

	// メッセージ欄
	CString mMessage;

public:
	// コマンド名
	CString mName;
	// 説明
	CString mDescription;
	// パス
	CString mPath;
	// メッセージ
	CString mNotifyMessage;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonFileBrowse();
	afx_msg void OnButtonDirBrowse();
};


}
}
}

