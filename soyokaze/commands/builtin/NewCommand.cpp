#include "pch.h"
#include "framework.h"
#include "commands/builtin/NewCommand.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "core/CommandRepository.h"
#include "IconLoader.h"
#include "CommandFile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;

CString NewCommand::GetType() { return _T("Builtin-New"); }

NewCommand::NewCommand(LPCTSTR name) :
	mRefCount(1)
{
	mName = name ? name : _T("new");
}

NewCommand::~NewCommand()
{
}

CString NewCommand::GetName()
{
	return mName;
}

CString NewCommand::GetDescription()
{
	return _T("【新規作成】");
}

BOOL NewCommand::Execute()
{
	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();
	cmdRepoPtr->NewCommandDialog();
	return TRUE;
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

CString NewCommand::GetErrorString()
{
	return _T("");
}

HICON NewCommand::GetIcon()
{
	return IconLoader::Get()->LoadNewIcon();
}

int NewCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool NewCommand::IsEditable()
{
	return false;
}


int NewCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command* NewCommand::Clone()
{
	return new NewCommand();
}

bool NewCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);
	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
	return true;
}

uint32_t NewCommand::AddRef()
{
	return ++mRefCount;
}

uint32_t NewCommand::Release()
{
	auto n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

