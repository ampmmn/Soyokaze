// あ
#pragma once

#include "control/SinglePageDialog.h"
#include "hotkey/HotKeyAttribute.h"

// 
class ExtraActionHotKeyDialog : public launcherapp::control::SinglePageDialog
{
public:
	ExtraActionHotKeyDialog(const HOTKEY_ATTR& attr, CWnd* parentWnd = nullptr);
	virtual ~ExtraActionHotKeyDialog();

	void GetAttribute(HOTKEY_ATTR& attr);

	HOTKEY_ATTR mHotKeyAttr;
	// 初期値
	HOTKEY_ATTR mHotKeyAttrInit;

	short mVK;
	// メッセージ欄
	CString mMessage;

	static bool ShowDialog(HOTKEY_ATTR& attr, CWnd* parent=nullptr);

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV サポート
	BOOL OnInitDialog() override;
	void OnOK() override;

	bool UpdateStatusForHotKey();

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void UpdateStatus();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

