#pragma once

#include "actions/core/ActionBase.h"

namespace launcherapp { namespace actions { namespace builtin {

// 指定されたパスをファイラで開くアクション
class OpenPathInFilerAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	OpenPathInFilerAction(const CString fullPath);
	~OpenPathInFilerAction();

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

private:
	CString mFullPath;
};



}}}

