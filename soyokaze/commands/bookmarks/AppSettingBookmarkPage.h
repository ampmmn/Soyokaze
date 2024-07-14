#pragma once

#include "gui/SettingPage.h"

namespace launcherapp {
namespace commands {
namespace bookmarks {

class AppSettingBookmarkPage : public SettingPage
{
public:
	AppSettingBookmarkPage(CWnd* parentWnd);
	virtual ~AppSettingBookmarkPage();

	// 機能を利用するか
	BOOL mIsEnableBookmarks;

	// URLを絞り込み対象とするか?
	BOOL mIsUseURL;

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
	afx_msg void OnCheckEnable();
};


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

