#include "pch.h"
#include "framework.h"
#include "ExitCommand.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ExitCommand::ExitCommand() : mRefCount(1)
{
}

ExitCommand::~ExitCommand()
{
}

CString ExitCommand::GetName()
{
	return _T("exit");
}

CString ExitCommand::GetDescription()
{
	return _T("【終了】");
}

BOOL ExitCommand::Execute()
{
	PostQuitMessage(0);
	return TRUE;
}

BOOL ExitCommand::Execute(const std::vector<CString>& args)
{
	// 引数指定しても動作はかわらない
	return Execute();
}

CString ExitCommand::GetErrorString()
{
	return _T("");
}

HICON ExitCommand::GetIcon()
{
	return IconLoader::Get()->LoadExitIcon();
}

int ExitCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

soyokaze::core::Command* ExitCommand::Clone()
{
	return new ExitCommand();
}

uint32_t ExitCommand::AddRef()
{
	return ++mRefCount;
}

uint32_t ExitCommand::Release()
{
	auto n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}
