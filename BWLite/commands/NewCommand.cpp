#include "pch.h"
#include "framework.h"
#include "commands/NewCommand.h"
#include "commands/ShellExecCommand.h"
#include "CommandMap.h"
#include "IconLoader.h"
#include "CommandEditDialog.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

NewCommand::NewCommand(CommandRepository* cmdMapPtr) : mCmdMapPtr(cmdMapPtr)
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
	return _T("yV‹Kì¬z");
}

BOOL NewCommand::Execute()
{
	mCmdMapPtr->NewCommandDialog(nullptr);
	return TRUE;
}

BOOL NewCommand::Execute(const std::vector<CString>& args)
{
	const CString* namePtr = nullptr;
	if (args.size() > 0) {
		namePtr = &args[0];
	}
	mCmdMapPtr->NewCommandDialog(namePtr);
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

BOOL NewCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

Command* NewCommand::Clone()
{
	return new NewCommand(mCmdMapPtr);
}

