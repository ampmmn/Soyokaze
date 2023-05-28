#include "pch.h"
#include "framework.h"
#include "commands/NewCommand.h"
#include "commands/ShellExecCommand.h"
#include "CommandRepository.h"
#include "IconLoader.h"
#include "gui/CommandEditDialog.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

NewCommand::NewCommand(CommandRepository* cmdMapPtr) :
	mCmdMapPtr(cmdMapPtr),
	mRefCount(1)
{
}

NewCommand::~NewCommand()
{
}

CString NewCommand::GetName()
{
	return _T("new");
}

CString NewCommand::GetDescription()
{
	return _T("【新規作成】");
}

BOOL NewCommand::Execute()
{
	mCmdMapPtr->NewCommandDialog(nullptr, nullptr);
	return TRUE;
}

BOOL NewCommand::Execute(const Parameter& param)
{
	std::vector<CString> args;
	param.GetParameters(args);

	const CString* namePtr = nullptr;
	if (args.size() > 0) {
		namePtr = &args[0];
	}
	const CString* pathPtr = nullptr;
	if (args.size() > 1) {
		pathPtr = &args[1];
	}
	mCmdMapPtr->NewCommandDialog(namePtr, pathPtr);
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

soyokaze::core::Command* NewCommand::Clone()
{
	return new NewCommand(mCmdMapPtr);
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
