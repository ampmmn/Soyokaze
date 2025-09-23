#include "pch.h"
#include "OpenURLAction.h"
#include "commands/common/SubProcess.h"
#include "externaltool/webbrowser/ConfiguredBrowserEnvironment.h"
#include "utility/Path.h"

using namespace launcherapp::commands::common;
using ConfiguredBrowserEnvironment = launcherapp::externaltool::webbrowser::ConfiguredBrowserEnvironment;

namespace launcherapp { namespace actions { namespace web {

OpenURLAction::OpenURLAction(const CString& url) :
 	mURL(url), mEnv(ConfiguredBrowserEnvironment::GetInstance())
{
}

OpenURLAction::OpenURLAction(const CString& url, BrowserEnvironment* env) :
 	mURL(url), mEnv(env)
{
}

OpenURLAction::~OpenURLAction()
{
	// mEnvは解放不要
}

void OpenURLAction::SetDisplayName(const CString& displayName)
{
	mDisplayName = displayName;
}

// Action
// アクションの内容を示す名称
CString OpenURLAction::GetDisplayName()
{
	if (mDisplayName.IsEmpty() == FALSE) {
		return mDisplayName;
	}
	else {
		if (mBrowserName.IsEmpty()) {
			mEnv->GetProductName(mBrowserName);
		}
		return mBrowserName + _T("でURLを開く");
	}
}

// アクションを実行する
bool OpenURLAction::Perform(Parameter* param, String* errMsg)
{
	CString browserPath;
	CString parameter;
	if (mEnv->GetInstalledExePath(browserPath) == false || mEnv->GetCommandlineParameter(parameter) == false) {
		// 無効なパスが設定されている場合はシステム設定に従って開く
		return ConfiguredBrowserEnvironment::GetInstance()->OpenURL(mURL);
	}

	// パスに空白を含む場合はダブルクォーテーションで囲む
	CString url(mURL);
	if (url.Find(_T(" ")) != -1) {
		url = _T("\"") + url + _T("\"");
	}
	CStringW params(parameter);
	params.Replace(_T("$target"), url);

	SubProcess::ProcessPtr process;
	SubProcess exec(param);
	exec.Run(browserPath, params, process);

	return true;
}

}}}

