#pragma once

#include "actions/core/ActionBase.h"

namespace launcherapp { namespace actions { namespace clipboard {

class CopyAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	CopyAction();
	~CopyAction();

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;
};



}}}

