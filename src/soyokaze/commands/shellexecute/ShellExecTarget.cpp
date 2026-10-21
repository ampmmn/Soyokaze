#include "pch.h"
#include "ShellExecTarget.h"

namespace launcherapp { namespace commands { namespace shellexecute {

ShellExecTarget::ShellExecTarget(const CommandParam& param): mParam(param)
{
}

// 実行時引数・マクロの展開は、実行時にSubProcessが行う
// (ここで展開すると、展開済みの値が再度展開される可能性がある)
CString ShellExecTarget::GetPath(Parameter* args)
{
	const auto& attr = SelectAttr(args);
	return attr.mPath;
}

CString ShellExecTarget::GetParameter(Parameter* args)
{
	const auto& attr = SelectAttr(args);
	return attr.mParam;
}

CString ShellExecTarget::GetWorkDir(Parameter* args)
{
	const auto& attr = SelectAttr(args);
	return attr.mDir;
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
