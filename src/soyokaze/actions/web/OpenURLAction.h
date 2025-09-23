#pragma once

#include "actions/core/ActionBase.h"
#include "externaltool/webbrowser/BrowserEnvironment.h"

namespace launcherapp { namespace actions { namespace web {

class OpenURLAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	using BrowserEnvironment = launcherapp::externaltool::webbrowser::BrowserEnvironment;

public:
	OpenURLAction(const CString& url);
	OpenURLAction(const CString& url, BrowserEnvironment* brwsEnv);
	~OpenURLAction();

	void SetDisplayName(const CString& displayName);

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

private:
	CString mURL;
	CString mBrowserName;
	CString mDisplayName;
	BrowserEnvironment* mEnv{nullptr};
};



}}}

