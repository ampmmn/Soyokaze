#include "pch.h"
#include "framework.h"
#include "commands/ManagerCommand.h"
#include "core/CommandRepository.h"
#include "IconLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ManagerCommand::ManagerCommand() :
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
	soyokaze::core::CommandRepository::GetInstance()->ManagerDialog();
	return TRUE;
}

BOOL ManagerCommand::Execute(const Parameter& param)
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
	return new ManagerCommand();
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
