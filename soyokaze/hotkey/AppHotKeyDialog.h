// あ
#pragma once

#include "gui/SinglePageDialog.h"
#include "hotkey/HotKeyAttribute.h"

// 
class AppHotKeyDialog : public launcherapp::gui::SinglePageDialog
{
public:
	AppHotKeyDialog(const HOTKEY_ATTR& attr, CWnd* parent = nullptr);
	virtual ~AppHotKeyDialog();

	void SetTargetName(const CString& name);

	void GetAttribute(HOTKEY_ATTR& attr);

	void SetModifierHotKeyType(bool isModifierHotKey);
	bool IsModifierHotKey();
	void SetModifierFirstVK(UINT vk);
	void SetModifierSecondVK(UINT vk);
	UINT GetModifierFirstVK();
	UINT GetModifierSecondVK();

	HOTKEY_ATTR mHotKeyAttr;
	// メッセージ欄
	CString mMessage;
	// 設定対象の名前
	CString mTargetName;
	//
	int mHotKeyType;
	// 補足説明
	CString mDescription;

	int mFirstVKIndex;
	int mSecondVKIndex;

	static CString ToString(UINT firstVK, UINT secondVK);

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
