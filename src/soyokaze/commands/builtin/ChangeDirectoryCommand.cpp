#include "pch.h"
#include "framework.h"
#include "commands/builtin/ChangeDirectoryCommand.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "commands/common/CommandParameterFunctions.h"
#include "icon/IconLoader.h"
#include "utility/Path.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

using namespace launcherapp::commands::common;

using ShellExecCommand = launcherapp::commands::shellexecute::ShellExecCommand;
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;


CString ChangeDirectoryCommand::TYPE(_T("Builtin-CD"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(ChangeDirectoryCommand)


CString ChangeDirectoryCommand::GetType()
{
	return TYPE;
}

ChangeDirectoryCommand::ChangeDirectoryCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("cd"))
{
	mDescription = _T("【カレントディレクトリ変更】");
	mCanSetConfirm = false;
	mCanDisable = true;
}

ChangeDirectoryCommand::ChangeDirectoryCommand(const ChangeDirectoryCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

ChangeDirectoryCommand::~ChangeDirectoryCommand()
{
}

BOOL ChangeDirectoryCommand::Execute(Parameter* param)
{
	auto pref = AppPreference::Get();

	// Ctrlキーがおされていた場合はカレントディレクトリをファイラで表示
	bool isOpenPath = GetModifierKeyState(param, MASK_CTRL) != 0;
	if (isOpenPath) {
		std::vector<TCHAR> currentDir(MAX_PATH_NTFS);
		GetCurrentDirectory(MAX_PATH_NTFS, &currentDir.front());
		_tcscat_s(&currentDir.front(), currentDir.size(), _T("\\"));

		ShellExecCommand cmd;
		cmd.SetPath(&currentDir.front());

		return cmd.Execute(CommandParameterBuilder::EmptyParam());
	}


	if (param->HasParameter() == false) {
		return TRUE;
	}

	LPCTSTR newDir = param->GetParam(0);
	if (Path::IsDirectory(newDir) == FALSE) {
		return TRUE;
	}

	// カレントディレクトリ変更
	SetCurrentDirectory(newDir);

	return TRUE;
}

HICON ChangeDirectoryCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-185);
}

launcherapp::core::Command* ChangeDirectoryCommand::Clone()
{
	return new ChangeDirectoryCommand(*this);
}

}
}
}
