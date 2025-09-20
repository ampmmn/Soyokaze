#pragma once

#include "actions/core/ActionBase.h"

namespace launcherapp { namespace actions { namespace web {

class OpenInChromeAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	OpenInChromeAction(const CString& url);
	~OpenInChromeAction();

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

	static bool GetChromeExecutablePath(LPTSTR path, size_t len);

private:
	CString mURL;
};



}}}

