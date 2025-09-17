#pragma once

#include "actions/core/ActionBase.h"

// なにもしないアクション
namespace launcherapp { namespace actions { namespace builtin {

class NullAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	NullAction() = default;
	~NullAction() = default;

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;
};



}}}

