#include "pch.h"
#include "framework.h"
#include "ReloadCommand.h"
#include "core/CommandRepository.h"
#include "CommandFile.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

CString ReloadCommand::GetType() { return _T("Builtin-Reload"); }

ReloadCommand::ReloadCommand(LPCTSTR name) :
	mRefCount(1)
{
	mName = name ? name : _T("reload");
}

ReloadCommand::~ReloadCommand()
{
}

CString ReloadCommand::GetName()
{
	return mName;
}

CString ReloadCommand::GetDescription()
{
	return _T("【設定のリロード】");
}

CString ReloadCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_BUILTIN);
	return TEXT_TYPE;
}

BOOL ReloadCommand::Execute()
{
	return soyokaze::core::CommandRepository::GetInstance()->Load();
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

bool ReloadCommand::IsEditable()
{
	return false;
}

int ReloadCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command* ReloadCommand::Clone()
{
	return new ReloadCommand();
}

bool ReloadCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);
	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
	return true;
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

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

