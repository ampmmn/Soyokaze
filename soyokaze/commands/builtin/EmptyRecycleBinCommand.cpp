#include "pch.h"
#include "framework.h"
#include "commands/builtin/EmptyRecycleBinCommand.h"
#include "core/CommandRepository.h"
#include "CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

CString EmptyRecycleBinCommand::TYPE(_T("Builtin-EmptyRecycleBin"));

CString EmptyRecycleBinCommand::GetType()
{
	return TYPE;
}

EmptyRecycleBinCommand::EmptyRecycleBinCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("emptyrecyclebin"))
{
	mDescription = _T("【ごみ箱を空にする】");
}

EmptyRecycleBinCommand::~EmptyRecycleBinCommand()
{
}

HICON EmptyRecycleBinCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-55);
}


BOOL EmptyRecycleBinCommand::Execute(const Parameter& param)
{
	SHEmptyRecycleBin(nullptr, nullptr, SHERB_NOCONFIRMATION);
	return TRUE;
}

soyokaze::core::Command* EmptyRecycleBinCommand::Clone()
{
	return new EmptyRecycleBinCommand();
}

}
}
}
