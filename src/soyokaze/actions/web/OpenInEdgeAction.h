#pragma once

#include "actions/core/ActionBase.h"

namespace launcherapp { namespace actions { namespace web {

class OpenInEdgeAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	OpenInEdgeAction(const CString& url);
	~OpenInEdgeAction();

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

private:
	CString mURL;
};



}}}

