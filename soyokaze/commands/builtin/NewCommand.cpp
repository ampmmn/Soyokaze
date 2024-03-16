#include "pch.h"
#include "framework.h"
#include "NewCommand.h"
#include "core/CommandRepository.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

CString NewCommand::TYPE(_T("Builtin-New"));

CString NewCommand::GetType()
{
	return TYPE;
}

NewCommand::NewCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("new"))
{
	mDescription = _T("【新規作成】");
}

NewCommand::~NewCommand()
{
}

BOOL NewCommand::Execute(const Parameter& param)
{
	Parameter inParam;

	bool hasParam = false;

	std::vector<CString> args;
	param.GetParameters(args);
	if (args.size() > 0) {
		inParam.SetNamedParamString(_T("COMMAND"), args[0]);
		hasParam = true;
	}
	const CString* pathPtr = nullptr;
	if (args.size() > 1) {
		inParam.SetNamedParamString(_T("PATH"), args[1]);
		hasParam = true;
	}
	if (hasParam) {
		inParam.SetNamedParamString(_T("TYPE"), _T("ShellExecuteCommand"));
	}

	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();
	cmdRepoPtr->NewCommandDialog(&inParam);

	return TRUE;
}

HICON NewCommand::GetIcon()
{
	return IconLoader::Get()->LoadNewIcon();
}

soyokaze::core::Command* NewCommand::Clone()
{
	return new NewCommand();
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

