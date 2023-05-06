#include "pch.h"
#include "framework.h"
#include "commands/ManagerCommand.h"
#include "CommandMap.h"
#include "IconLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ManagerCommand::ManagerCommand(CommandMap* cmdMapPtr) :
	mCmdMapPtr(cmdMapPtr)
{
}

ManagerCommand::~ManagerCommand()
{
}

CString ManagerCommand::GetName()
{
	return _T("manager");
}

CString ManagerCommand::GetDescription()
{
	return _T("【キーワードマネージャ】");
}

BOOL ManagerCommand::Execute()
{
	mCmdMapPtr->ManagerDialog();
	return TRUE;
}

BOOL ManagerCommand::Execute(const std::vector<CString>& args)
{
	// このコマンドは引数を使わない
	return Execute();
}

CString ManagerCommand::GetErrorString()
{
	return _T("");
}

HICON ManagerCommand::GetIcon()
{
	return IconLoader::Get()->LoadNewIcon();
}

BOOL ManagerCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

Command* ManagerCommand::Clone()
{
	return new ManagerCommand(mCmdMapPtr);
}

