#pragma once

#include "gui/SettingPage.h"
#include "hotkey/HotKeyDialog.h"

class InputWindowKeySettingPage : public SettingPage
{
public:
	InputWindowKeySettingPage(CWnd* parentWnd);
	virtual ~InputWindowKeySettingPage();

	// 上へ
	CString mHotKeyUp;
	HOTKEY_ATTR mHotKeyAttrUp;
	// 下へ
	CString mHotKeyDown;
	HOTKEY_ATTR mHotKeyAttrDown;
	// 決定
	CString mHotKeyEnter;
	HOTKEY_ATTR mHotKeyAttrEnter;
	// 補完
	CString mHotKeyCompl;
	HOTKEY_ATTR mHotKeyAttrCompl;
	// コピー
	CString mHotKeyCopy;
	HOTKEY_ATTR mHotKeyAttrCopy;

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
	afx_msg void OnButtonHotKeyUp();
	afx_msg void OnButtonHotKeyDown();
	afx_msg void OnButtonHotKeyEnter();
	afx_msg void OnButtonHotKeyCompl();
	afx_msg void OnButtonHotKeyCopy();
	afx_msg void OnButtonReset();
};

