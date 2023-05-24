#include "pch.h"
#include "framework.h"
#include "commands/ManagerCommand.h"
#include "CommandRepository.h"
#include "IconLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ManagerCommand::ManagerCommand(CommandRepository* cmdMapPtr) :
	mCmdMapPtr(cmdMapPtr),
	mRefCount(1)
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

int ManagerCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

soyokaze::core::Command* ManagerCommand::Clone()
{
	return new ManagerCommand(mCmdMapPtr);
}

uint32_t ManagerCommand::AddRef()
{
	return ++mRefCount;
}

uint32_t ManagerCommand::Release()
{
	auto n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}
