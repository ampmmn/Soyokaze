#pragma once

#include "actions/builtin/ExecuteAction.h"
#include "commands/regexp/RegExpCommandParam.h"

namespace launcherapp { namespace commands { namespace regexp {

class RegExpExecutionTarget : public launcherapp::actions::builtin::ExecutionTarget
{
public:
	RegExpExecutionTarget(int matchLevel, const CommandParam& param);

	CString GetPath(Parameter* args) override;
	CString GetParameter(Parameter* args) override;
	CString GetWorkDir(Parameter* args) override;
	int GetShowType(Parameter* args) override;

	bool GetRegex(tregex& regexObject);
private:
	int mMatchLevel;
	CommandParam mParam;
};

}}}




