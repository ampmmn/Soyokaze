#include "pch.h"
#include "framework.h"
#include "ExitCommand.h"
#include "IconLoader.h"
#include "CommandFile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString ExitCommand::GetType() { return _T("Builtin-Exit"); }

ExitCommand::ExitCommand(LPCTSTR name) : mRefCount(1)
{
	mName = name ? name : _T("exit");
}

ExitCommand::~ExitCommand()
{
}

CString ExitCommand::GetName()
{
	return mName;
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

BOOL ExitCommand::Execute(const Parameter& param)
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

bool ExitCommand::IsEditable()
{
	return false;
}

int ExitCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command* ExitCommand::Clone()
{
	return new ExitCommand();
}

bool ExitCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);
	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
	return true;
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
