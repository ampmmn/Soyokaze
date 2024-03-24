// あ
#pragma once

#include "hotkey/HotKeyAttribute.h"

// 
class CommandHotKeyDialog : public CDialogEx
{
public:
	CommandHotKeyDialog(const HOTKEY_ATTR& attr, bool isGlobal);
	virtual ~CommandHotKeyDialog();

	void GetAttribute(HOTKEY_ATTR& attr);
	bool IsGlobal();

	HOTKEY_ATTR mHotKeyAttr;
	// 初期値
	HOTKEY_ATTR mHotKeyAttrInit;

	BOOL mIsGlobal;

	// メッセージ欄
	CString mMessage;

protected:
	static bool IsReservedKey(const HOTKEY_ATTR& attr);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	void UpdateStatus();

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonClear();
};

