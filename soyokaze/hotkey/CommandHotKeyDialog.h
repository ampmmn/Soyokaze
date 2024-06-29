// あ
#pragma once

#include "gui/SinglePageDialog.h"
#include "hotkey/CommandHotKeyAttribute.h"

// 
class CommandHotKeyDialog : public launcherapp::gui::SinglePageDialog
{
public:
	CommandHotKeyDialog(const CommandHotKeyAttribute& attr);
	virtual ~CommandHotKeyDialog();

	void SetTargetName(const CString& name);

	void GetAttribute(CommandHotKeyAttribute& attr);

	CommandHotKeyAttribute mHotKeyAttr;
	// 初期値
	CommandHotKeyAttribute mHotKeyAttrInit;

	// メッセージ欄
	CString mMessage;
	//
	CString mTargetName;

protected:
	static bool IsReservedKey(const HOTKEY_ATTR& attr);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void UpdateStatus();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonClear();
};

