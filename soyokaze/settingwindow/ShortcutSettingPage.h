#pragma once

#include "gui/SettingPage.h"

class ShortcutSettingPage : public SettingPage
{
public:
	ShortcutSettingPage(CWnd* parentWnd);
	virtual ~ShortcutSettingPage();


	static void CreateStartMenuPath(CString& pathToMenu);
	static bool IsStartMenuExists();
	static bool CreateStartMenu();

public:
	CString mSoyokazePath;

	// 各種ショートカットのパス
	CString mSendToPath;
	CString mStartMenuDir;
	CString mStartMenuPath;
	CString mDesktopPath;
	CString mStartupPath;

	BOOL mSendTo;
	BOOL mStartMenu;
	BOOL mDesktop;
	BOOL mStartup;


protected:
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	BOOL OnKillActive() override;
	BOOL OnSetActive() override;
	void OnEnterSettings() override;

	void UpdateStatus();
// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonDelete();
};

