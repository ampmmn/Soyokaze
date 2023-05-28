#include "pch.h"
#include "framework.h"
#include "ReloadCommand.h"
#include "CommandRepository.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ReloadCommand::ReloadCommand(CommandRepository* pMap) :
	mCmdMapPtr(pMap),
	mRefCount(1)
{
}

ReloadCommand::~ReloadCommand()
{
}

CString ReloadCommand::GetName()
{
	return _T("reload");
}

CString ReloadCommand::GetDescription()
{
	return _T("【設定のリロード】");
}

BOOL ReloadCommand::Execute()
{
	return mCmdMapPtr->Load();
}

BOOL ReloadCommand::Execute(const Parameter& param)
{
	// 引数指定しても動作はかわらない
	return Execute();
}

CString ReloadCommand::GetErrorString()
{
	return _T("");
}

HICON ReloadCommand::GetIcon()
{
	return IconLoader::Get()->LoadReloadIcon();
}

int ReloadCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

soyokaze::core::Command* ReloadCommand::Clone()
{
	return new ReloadCommand(mCmdMapPtr);
}

uint32_t ReloadCommand::AddRef()
{
	return ++mRefCount;
}

uint32_t ReloadCommand::Release()
{
	auto n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}
