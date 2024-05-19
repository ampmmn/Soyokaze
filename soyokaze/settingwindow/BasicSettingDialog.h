#pragma once

#include "gui/SettingPage.h"
#include "hotkey/HotKeyAttribute.h"
// 
class BasicSettingDialog : public SettingPage
{
public:
	BasicSettingDialog(CWnd* parentWnd);
	virtual ~BasicSettingDialog();

	// ランチャー呼び出しキー（表示用)
	CString mHotKey;
	HOTKEY_ATTR mHotKeyAttr;

	//
	bool mIsModifierHotKey;
	UINT mModifierFirstVK;
	UINT mModifierSecondVK;

	// 表示中にホットキーを押したら隠れる
	BOOL mIsShowToggle;

	// 入力画面を非表示にするときに入力文字列を消去しない
	BOOL mIsKeepTextWhenDlgHide;


	// 起動直後は入力画面を非表示にする
	BOOL mIsHideOnRun;

protected:
	bool UpdateStatus();

	BOOL OnKillActive() override;
	BOOL OnSetActive() override;
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	void OnEnterSettings() override;
	bool GetHelpPageId(CString& id) override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonHotKey();
};

