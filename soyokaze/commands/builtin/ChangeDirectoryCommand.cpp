#include "pch.h"
#include "framework.h"
#include "commands/builtin/ChangeDirectoryCommand.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "core/CommandRepository.h"
#include "AppPreference.h"
#include "CommandFile.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;

CString ChangeDirectoryCommand::GetType() { return _T("Builtin-CD"); }

ChangeDirectoryCommand::ChangeDirectoryCommand(LPCTSTR name) : mRefCount(1)
{
	mName = name ? name : _T("cd");
}

ChangeDirectoryCommand::~ChangeDirectoryCommand()
{
}

CString ChangeDirectoryCommand::GetName()
{
	return mName;
}

CString ChangeDirectoryCommand::GetDescription()
{
	return _T("【カレントディレクトリ変更】");
}

BOOL ChangeDirectoryCommand::Execute()
{
	Parameter param;
	return Execute(param);
}

BOOL ChangeDirectoryCommand::Execute(const Parameter& param)
{
	auto pref = AppPreference::Get();

	// Ctrlキーがおされていた場合はカレントディレクトリをファイラで表示
	bool isOpenPath = pref->IsShowFolderIfCtrlKeyIsPressed() &&
	                  param.GetNamedParamBool(_T("CtrlKeyPressed"));
	if (isOpenPath) {
		TCHAR currentDir[MAX_PATH_NTFS];
		GetCurrentDirectory(MAX_PATH_NTFS, currentDir);
		_tcscat_s(currentDir, _T("\\"));

		ShellExecCommand cmd;
		cmd.SetPath(currentDir);
		return cmd.Execute();
	}

	std::vector<CString> args;
	param.GetParameters(args);

	if (args.empty()) {
		return TRUE;
	}

	if (PathIsDirectory(args[0]) == FALSE) {
		return TRUE;
	}

	// カレントディレクトリ変更
	SetCurrentDirectory(args[0]);

	return TRUE;
}

CString ChangeDirectoryCommand::GetErrorString()
{
	return _T("");
}

HICON ChangeDirectoryCommand::GetIcon()
{
	return IconLoader::Get()->LoadDefaultIcon();
}

int ChangeDirectoryCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool ChangeDirectoryCommand::IsEditable()
{
	return false;
}

int ChangeDirectoryCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command* ChangeDirectoryCommand::Clone()
{
	return new ChangeDirectoryCommand();
}

bool ChangeDirectoryCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);
	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());
	return true;
}

uint32_t ChangeDirectoryCommand::AddRef()
{
	return ++mRefCount;
}

uint32_t ChangeDirectoryCommand::Release()
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
