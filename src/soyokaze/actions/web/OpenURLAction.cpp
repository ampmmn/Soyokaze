#include "pch.h"
#include "OpenURLAction.h"
#include "commands/common/SubProcess.h"
#include "commands/common/ExpandFunctions.h"
#include "externaltool/webbrowser/ConfiguredBrowserEnvironment.h"
#include "utility/CommandLineArgument.h"
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
bool OpenURLAction::Perform(Parameter* param, String*)
{
	CString browserPath;
	CString parameter;
	if (mEnv->GetInstalledExePath(browserPath) == false || mEnv->GetCommandlineParameter(parameter) == false) {
		// 無効なパスが設定されている場合はシステム設定に従って開く
		return ConfiguredBrowserEnvironment::GetInstance()->OpenURL(mURL);
	}

	// URLを展開する
	// (展開済みの値は再度解釈されてはならないため、ここでのみ展開する)
	CString url(mURL);
	ExpandArgumentsAndMacros(url, param, ArgumentQuoting::None, true);

	// URLはコマンドラインとして渡すため、引用符を付けて渡す
	CStringW params(parameter);
	launcherapp::utility::ReplaceQuotedPlaceholder(params, _T("$target"), url);

	SubProcess::ProcessPtr process;
	SubProcess exec(param);
	// パラメータは既に展開済みのため、実行時に再度展開しない
	exec.SetExpanded(true);
	exec.Run(browserPath, params, process);

	return true;
}

}}}

