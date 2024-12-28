#include "pch.h"
#include "framework.h"
#include "commands/builtin/EmptyRecycleBinCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

CString EmptyRecycleBinCommand::TYPE(_T("Builtin-EmptyRecycleBin"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(EmptyRecycleBinCommand)

CString EmptyRecycleBinCommand::GetType()
{
	return TYPE;
}

EmptyRecycleBinCommand::EmptyRecycleBinCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("emptyrecyclebin"))
{
	mDescription = _T("【ごみ箱を空にする】");
	mCanSetConfirm = true;
	mCanDisable = true;
}

EmptyRecycleBinCommand::EmptyRecycleBinCommand(const EmptyRecycleBinCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

EmptyRecycleBinCommand::~EmptyRecycleBinCommand()
{
}

HICON EmptyRecycleBinCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-55);
}


BOOL EmptyRecycleBinCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	DWORD flags = mIsConfirmBeforeRun ? 0 : SHERB_NOCONFIRMATION;
	SHEmptyRecycleBin(nullptr, nullptr, flags);
	return TRUE;
}

launcherapp::core::Command* EmptyRecycleBinCommand::Clone()
{
	return new EmptyRecycleBinCommand(*this);
}

}
}
}
