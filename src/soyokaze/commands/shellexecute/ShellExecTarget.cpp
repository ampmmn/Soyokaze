#include "pch.h"
#include "ShellExecTarget.h"
#include "commands/common/ExpandFunctions.h"

using namespace launcherapp::commands::common;

namespace launcherapp { namespace commands { namespace shellexecute {

ShellExecTarget::ShellExecTarget(const CommandParam& param): mParam(param)
{
}

CString ShellExecTarget::GetPath(Parameter* args)
{
	const auto& attr = SelectAttr(args);
	auto path = attr.mPath;
	ExpandArguments(path, args);
	ExpandMacros(path);
	return path;
}

CString ShellExecTarget::GetParameter(Parameter* args)
{
	const auto& attr = SelectAttr(args);
	auto param = attr.mParam;
	ExpandArguments(param, args);
	ExpandMacros(param);
	return param;
}

CString ShellExecTarget::GetWorkDir(Parameter* args)
{
	const auto& attr = SelectAttr(args);
	auto workDir = attr.mDir;
	ExpandArguments(workDir, args);
	ExpandMacros(workDir);
	return workDir;
}

int ShellExecTarget::GetShowType(Parameter* args)
{
	const auto& attr = SelectAttr(args);
	return attr.GetShowType();
}

const ATTRIBUTE& ShellExecTarget::SelectAttr(Parameter* args)
{
	if (mParam.mIsUse0 == FALSE) {
		return mParam.mNormalAttr;
	}
	return args->HasParameter() ? mParam.mNormalAttr : mParam.mNoParamAttr;
}


}}}
