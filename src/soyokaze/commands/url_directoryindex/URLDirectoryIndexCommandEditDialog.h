#pragma once

#include "control/SinglePageDialog.h"
#include <memory>
#include "commands/url_directoryindex/URLDirectoryIndexCommandParam.h"
#include "hotkey/CommandHotKeyAttribute.h"

class IconLabel;

namespace launcherapp {
namespace commands {
namespace url_directoryindex {


class URLDirectoryIndexCommandEditDialog : public launcherapp::control::SinglePageDialog
{
public:
	URLDirectoryIndexCommandEditDialog(CWnd* parentWnd = nullptr);
	virtual ~URLDirectoryIndexCommandEditDialog();

	void SetName(const CString& name);
	void SetOriginalName(const CString& name);

	void SetParam(const CommandParam& param);
	const CommandParam& GetParam();

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	bool Overlap(CWnd* dstWnd, CWnd* srcWnd);

	// 編集開始時のコマンド名
	CString mOrgName;

	// メッセージ欄
	CString mMessage;

public:
	CommandParam mParam;

	// ホットキー(表示用)
	CString mHotKey;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonHotKey();
};

} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp

