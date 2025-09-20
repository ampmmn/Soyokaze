#pragma once

#include "actions/core/ActionBase.h"

namespace launcherapp { namespace actions { namespace builtin {

class RegisterNewCommandAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	RegisterNewCommandAction(bool useFirstTokenOnly);
	~RegisterNewCommandAction() = default;

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

private:
	bool mUseFirstTonenOnly;
};



}}}

