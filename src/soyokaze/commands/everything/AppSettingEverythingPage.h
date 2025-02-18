#pragma once

#include "gui/SettingPage.h"

namespace launcherapp {
namespace commands {
namespace everything {

class AppSettingEverythingPage : public SettingPage
{
public:
	AppSettingEverythingPage(CWnd* parentWnd);
	virtual ~AppSettingEverythingPage();

	BOOL mIsUseAPI;
	BOOL mIsRunApp;

	// コマンドライン経由で使用する場合のEverything.exeのパス
	CString mEverythingExePath;
	
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
	afx_msg void OnUpdateStatus();
	afx_msg void OnButtonBrowse();
	afx_msg void OnNotifyLinkOpen(NMHDR *pNMHDR, LRESULT *pResult);
};


} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

