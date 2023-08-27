#include "pch.h"
#include "framework.h"
#include "commands/builtin\MainDirCommand.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "utility/AppProfile.h"
#include "IconLoader.h"
#include "CommandFile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {


using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;

CString MainDirCommand::GetType() { return _T("Builtin-MainDir"); }

MainDirCommand::MainDirCommand(LPCTSTR name) : mRefCount(1)
{
	mName = name ? name : _T("maindir");
}

MainDirCommand::~MainDirCommand()
{
}

CString MainDirCommand::GetName()
{
	return mName;
}

CString MainDirCommand::GetDescription()
{
	return _T("【メインフォルダ】");
}

CString MainDirCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_BUILTIN);
	return TEXT_TYPE;
}

BOOL MainDirCommand::Execute(const Parameter& param)
{
	TCHAR mainDirPath[65536];
	GetModuleFileName(NULL, mainDirPath, 65536);
	PathRemoveFileSpec(mainDirPath);
	_tcscat_s(mainDirPath, _T("\\"));

	ShellExecCommand cmd;
	cmd.SetPath(mainDirPath);

	Parameter paramEmpty;
	return cmd.Execute(paramEmpty);
}

CString MainDirCommand::GetErrorString()
{
	return _T("");
}

HICON MainDirCommand::GetIcon()
{
	return IconLoader::Get()->LoadMainDirIcon();
}

int MainDirCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool MainDirCommand::IsEditable()
{
	return false;
}


int MainDirCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command* MainDirCommand::Clone()
{
	return new MainDirCommand();
}

bool MainDirCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);
	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
	return true;
}

uint32_t MainDirCommand::AddRef()
{
	return ++mRefCount;
}

uint32_t MainDirCommand::Release()
{
	auto n = --mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

}
}
}
