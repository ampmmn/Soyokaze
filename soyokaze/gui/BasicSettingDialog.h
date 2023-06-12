#pragma once

#include "gui/SettingPage.h"
#include "gui/HotKeyDialog.h"

// 
class BasicSettingDialog : public SettingPage
{
public:
	BasicSettingDialog(CWnd* parentWnd);
	virtual ~BasicSettingDialog();

	// ランチャー呼び出しキー（表示用)
	CString mHotKey;
	HOTKEY_ATTR mHotKeyAttr;

	BOOL mIsShowToggle;

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

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonHotKey();
};

