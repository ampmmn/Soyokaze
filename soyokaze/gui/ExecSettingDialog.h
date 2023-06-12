#pragma once

#include "gui/SettingPage.h"

// 
class ExecSettingDialog : public SettingPage
{
public:
	ExecSettingDialog(CWnd* parentWnd);
	virtual ~ExecSettingDialog();


	// フォルダを開くファイラーを指定
	BOOL mIsUseExternalFiler;
	// ファイラーのパス
	CString mFilerPath;
	// ファイラーのパラメータ
	CString mFilerParam;

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
	afx_msg void OnButtonBrowseFile();
	afx_msg void OnCheckUseFilter();
};

