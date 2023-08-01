#pragma once

#include "gui/SettingPage.h"

// 
class ExtensionSettingDialog : public SettingPage
{
public:
	ExtensionSettingDialog(CWnd* parentWnd);
	virtual ~ExtensionSettingDialog();

	BOOL mIsEnableCalc;
	CString mPythonDLLPath;

	// ウインドウタイトルによるウインドウ切り替え機能
	BOOL mIsEnableWindowTitle;
	// Excelワークシート名によるウインドウ切り替え機能
	BOOL mIsEnableWorksheet;
	// ブックマーク選択機能
	BOOL mIsEnableBookmarks;
	// コントロールパネル選択機能
	BOOL mIsEnableControlPanel;

protected:
	bool UpdateStatus();

	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	BOOL OnKillActive() override;
	BOOL OnSetActive() override;
	void OnEnterSettings() override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBrowsePyhonDLLPath();
	afx_msg void OnCheckEnableCalculator();
};

