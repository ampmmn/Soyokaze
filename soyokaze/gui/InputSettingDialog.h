#pragma once

#include "gui/SettingPage.h"

class InputSettingDialog : public SettingPage
{
public:
	InputSettingDialog(CWnd* parentWnd);
	virtual ~InputSettingDialog();

	// 入力画面を表示するときにIMEをオフにする
	BOOL mIsIMEOff;

	// ネットワークパスを無視する
	BOOL mIsIgnoreUNC;

	// C/Migemo検索を有効にする
	BOOL mIsEnableMigemo;

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
	afx_msg void OnNotifyLinkOpen(NMHDR *pNMHDR, LRESULT *pResult);
};

