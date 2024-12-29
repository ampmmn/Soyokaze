#pragma once

#include <memory>
#include "gui/SettingPage.h"
#include "commands/shellexecute/ShellExecCommandParam.h"

class IconLabel;

class CommandEditDialog : public SettingPage
{
	using CommandParam = launcherapp::commands::shellexecute::CommandParam;

public:
	CommandEditDialog(CWnd* parentWnd);
	virtual ~CommandEditDialog();

	void SetOriginalName(const CString& name);

	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	void ResolveShortcut(CString& path);
	bool IsEditableFileType(CString path);

	// 編集開始時のコマンド名
	CString mOrgName;

	// メッセージ欄
	CString mMessage;

public:
	CommandParam mParam;

	std::unique_ptr<IconLabel> mIconLabelPtr;

	// ホットキー(表示用)
	CString mHotKey;

	// アイコン(表示用)
	HICON mIcon;

	// 
	CMFCMenuButton mPathMenuBtn;
	CMenu mMenuForPathBtn;

// 実装
protected:
	BOOL OnKillActive() override;
	BOOL OnSetActive() override;
	void OnEnterSettings() override;
	bool GetHelpPageId(CString& id) override;

	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdateStatus();
	afx_msg void OnUpdatePath();
	afx_msg void OnButtonBrowseFile1Clicked();
	afx_msg void OnButtonBrowseDir1Clicked();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonHotKey();
	afx_msg void OnButtonResolveShortcut();
	afx_msg LRESULT OnUserMessageIconChanged(WPARAM wp, LPARAM lp);
	afx_msg void OnNotifyLinkOpen(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnPathMenuBtnClicked();
	afx_msg void OnButtonBrowseDir3Clicked();
	void OpenTarget();
};

