#pragma once

#include "gui/SettingPage.h"

class InputSettingDialog : public SettingPage
{
public:
	InputSettingDialog(CWnd* parentWnd);
	virtual ~InputSettingDialog();

	// 絞込方法(0:前方一致 1:部分一致 2:スキップマッチング)
	int mMatchLevel;

	// 入力画面を表示するときにIMEをオフにする
	BOOL mIsIMEOff;

	// ネットワークパスを無視する
	BOOL mIsIgnoreUNC;

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
};

