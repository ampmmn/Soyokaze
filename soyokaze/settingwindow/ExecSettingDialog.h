#pragma once

#include "gui/SettingPage.h"

// 
class ExecSettingDialog : public SettingPage
{
public:
	ExecSettingDialog(CWnd* parentWnd);
	virtual ~ExecSettingDialog();


	// Ctrl+Enterキー実行でフォルダ表示する
	BOOL mIsShowFolderIfCtrlPressed;
	// フォルダを開くファイラーを指定
	BOOL mIsUseExternalFiler;
	// ファイル名を指定して実行を使用する
	BOOL mIsEnablePathFind;
	// ファイラーのパス
	CString mFilerPath;
	// ファイラーのパラメータ
	CString mFilerParam;

	// 同一フィルタコマンドの並列実行を許可する
	BOOL mIsArrowFilterConcurrentRun;

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
	afx_msg void OnButtonBrowseFile();
	afx_msg void OnCheckUseFilter();
};

