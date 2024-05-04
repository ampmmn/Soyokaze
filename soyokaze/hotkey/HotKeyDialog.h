// あ
#pragma once

#include "gui/SinglePageDialog.h"
#include "hotkey/HotKeyAttribute.h"

// 
class HotKeyDialog : public launcherapp::gui::SinglePageDialog
{
public:
	HotKeyDialog(const HOTKEY_ATTR& attr, CWnd* parent = nullptr);
	virtual ~HotKeyDialog();

	void SetTargetName(const CString& name);

	void GetAttribute(HOTKEY_ATTR& attr);

	HOTKEY_ATTR mHotKeyAttr;
	// メッセージ欄
	CString mMessage;
	// 設定対象の名前
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
};

