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

	bool IsEnableHotKey();
	bool IsEnableModifierHotKey();
	bool IsEnableModifierHotKeyOnRD();
	void SetEnableHotKey(bool isEnable);
	void SetEnableModifierHotKey(bool isEnable);
	void SetEnableModifierHotKeyOnRD(bool isEnable);
	void SetModifierFirstVK(UINT vk);
	void SetModifierSecondVK(UINT vk);
	UINT GetModifierFirstVK();
	UINT GetModifierSecondVK();

	HOTKEY_ATTR mHotKeyAttr;
	// メッセージ欄
	CString mMessage;

	BOOL mIsEnableHotKey;
	BOOL mIsEnableModifieHotKey;
	// リモートデスクトップにフォーカスがある場合であっても、修飾キーの組み合わせ入力によるホットキーを有効にする
	BOOL mIsEnableOnRD;
	// 補足説明
	CString mDescription;

	int mFirstVKIndex;
	int mSecondVKIndex;

	static CString ToString(UINT firstVK, UINT secondVK);

protected:
	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

