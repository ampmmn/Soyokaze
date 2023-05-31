#include "pch.h"
#include "framework.h"
#include "commands/ManagerCommand.h"
#include "core/CommandRepository.h"
#include "CommandFile.h"
#include "IconLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString ManagerCommand::GetType() { return _T("Builtin-Manager"); }

ManagerCommand::ManagerCommand(LPCTSTR name) :
	mRefCount(1)
{
	mName = name ? name : _T("manager");
}

ManagerCommand::~ManagerCommand()
{
}

CString ManagerCommand::GetName()
{
	return mName;
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

bool ManagerCommand::IsEditable()
{
	return false;
}


int ManagerCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command* ManagerCommand::Clone()
{
	return new ManagerCommand();
}

bool ManagerCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);
	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
	return true;
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
