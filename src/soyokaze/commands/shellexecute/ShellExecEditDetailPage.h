#pragma once

#include "gui/SettingPage.h"
#include "commands/shellexecute/ShellExecCommandParam.h"

class IconLabel;

class ShellExecEditDetailPage : public SettingPage
{
	using CommandParam = launcherapp::commands::shellexecute::CommandParam;

public:
	ShellExecEditDetailPage(CWnd* parentWnd);
	virtual ~ShellExecEditDetailPage();

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	void ResolveShortcut(CString& path);

	// メッセージ欄
	CString mMessage;

public:
	CommandParam mParam;

// 実装
protected:
	BOOL OnKillActive() override;
	BOOL OnSetActive() override;
	void OnEnterSettings() override;
	bool GetHelpPageId(CString& id) override;

	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnEditPath0Changed();
	afx_msg void OnButtonBrowseFile2Clicked();
	afx_msg void OnButtonBrowseDir2Clicked();
	afx_msg void OnButtonBrowseDir3Clicked();
	afx_msg void OnUpdateStatus();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonResolveShortcut0();
};

