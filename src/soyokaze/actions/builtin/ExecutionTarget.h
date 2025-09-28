#pragma once

#include "actions/core/ActionParameter.h"


namespace launcherapp { namespace actions { namespace builtin {

// ExecuteAction等が実行対象とする実行ファイルのパスとパラメータを取得するためのインタフェース
// ExecuteAction等を使う側でパラメータに応じてパスをカスタマイズしたいときにこのインタフェースを使う
class ExecutionTarget
{
public:
	using Parameter = launcherapp::actions::core::Parameter;

	virtual ~ExecutionTarget() = default;

	virtual CString GetPath(Parameter* args) = 0;
	virtual CString GetParameter(Parameter* args) = 0;
	virtual CString GetWorkDir(Parameter* args) = 0;
	virtual int GetShowType(Parameter* args) = 0;
};

}}}

