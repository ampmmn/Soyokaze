#pragma once

#include "gui/SettingPage.h"

namespace launcherapp {
namespace commands {
namespace pathconvert {

class AppSettingPathConvertPage : public SettingPage
{
public:
	AppSettingPathConvertPage(CWnd* parentWnd);
	virtual ~AppSettingPathConvertPage();

	// git-bashパス変換を利用するか?
	BOOL mIsEnableGitBashPath;
	// file://プロトコル→ローカルパス変換を利用するか?
	BOOL mIsEnableFileProtolPath;

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
};


} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp

