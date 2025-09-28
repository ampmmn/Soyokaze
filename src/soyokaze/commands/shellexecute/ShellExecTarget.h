#pragma once

#include "actions/builtin/ExecuteAction.h"
#include "commands/shellexecute/ShellExecCommandParam.h"

namespace launcherapp { namespace commands { namespace shellexecute {

class ShellExecTarget : public launcherapp::actions::builtin::ExecutionTarget
{
public:
	ShellExecTarget(const CommandParam& param);
	CString GetPath(Parameter* args) override;
	CString GetParameter(Parameter* args) override;
	CString GetWorkDir(Parameter* args) override;
	int GetShowType(Parameter* args) override;

private:
	const ATTRIBUTE& SelectAttr(Parameter* args);

private:
	CommandParam mParam;
};


}}}
