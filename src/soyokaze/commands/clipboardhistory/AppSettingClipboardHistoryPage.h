#pragma once

#include "gui/SettingPage.h"

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

class AppSettingClipboardHistoryPage : public SettingPage
{
public:
	AppSettingClipboardHistoryPage(CWnd* parentWnd);
	virtual ~AppSettingClipboardHistoryPage();

	BOOL mIsEnable;
	CString mPrefix;
	int mNumOfResults;
	int mSizeLimit;
	int mCountLimit;
	int mInterval;
	CString mExcludePattern;

	
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
};


} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp

