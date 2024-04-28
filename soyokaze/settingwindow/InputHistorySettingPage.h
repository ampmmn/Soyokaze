#pragma once

#include "gui/SettingPage.h"

class InputHistorySettingPage : public SettingPage
{
public:
	InputHistorySettingPage(CWnd* parentWnd);
	virtual ~InputHistorySettingPage();

	// 履歴機能を使う
	BOOL mIsUseHistory;
	// 履歴件数
	int mHistoryLimit;

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
	afx_msg void OnButtonClear();
	afx_msg void OnUpdateData();
};

