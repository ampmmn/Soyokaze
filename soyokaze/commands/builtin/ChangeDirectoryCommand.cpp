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


CString ChangeDirectoryCommand::TYPE(_T("Builtin-CD"));

CString ChangeDirectoryCommand::GetType()
{
	return TYPE;
}

ChangeDirectoryCommand::ChangeDirectoryCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("cd"))
{
	mDescription = _T("【カレントディレクトリ変更】");
}

ChangeDirectoryCommand::~ChangeDirectoryCommand()
{
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

		Parameter paramEmpty;
		return cmd.Execute(paramEmpty);
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

HICON ChangeDirectoryCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-185);
}

soyokaze::core::Command* ChangeDirectoryCommand::Clone()
{
	return new ChangeDirectoryCommand();
}

}
}
}
