#pragma once

#include "gui/SettingPage.h"
#include "gui/HotKeyDialog.h"

// 
class ViewSettingDialog : public SettingPage
{
public:
	ViewSettingDialog(CWnd* parentWnd);
	virtual ~ViewSettingDialog();

	// 入力画面を常に最前面に表示
	BOOL mIsTopMost;

	// アクティブ状態でなくなったらウインドウを隠す
	BOOL mIsHideOnInactive;

	// 半透明の表示方法
	int mTransparencyType;
	// 半透明表示の透明度
	UINT mAlpha;

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
	afx_msg void OnCbnTransparencyChanged();
};

