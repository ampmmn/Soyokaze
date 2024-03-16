#pragma once

#include "hotkey/HotKeyAttribute.h"

// 
class HotKeyDialog : public CDialogEx
{
public:
	HotKeyDialog(const HOTKEY_ATTR& attr);
	virtual ~HotKeyDialog();

	void GetAttribute(HOTKEY_ATTR& attr);

	HOTKEY_ATTR mHotKeyAttr;
	// メッセージ欄
	CString mMessage;

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

