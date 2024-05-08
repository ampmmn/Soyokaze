#include "pch.h"
#include "framework.h"
#include "commands/builtin/DisplayOffCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <powrprof.h>
#pragma comment(lib, "powrprof.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

CString DisplayOffCommand::TYPE(_T("Builtin-DisplayOff"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(DisplayOffCommand)

CString DisplayOffCommand::GetType()
{
	return TYPE;
}

DisplayOffCommand::DisplayOffCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("displayoff"))
{
	mDescription = _T("【モニターの電源をオフにする】");
	mCanSetConfirm = true;
	mCanDisable = true;
}

DisplayOffCommand::DisplayOffCommand(const DisplayOffCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

DisplayOffCommand::~DisplayOffCommand()
{
}

HICON DisplayOffCommand::GetIcon()
{
	// 消えているモニタのようなアイコン
	return IconLoader::Get()->GetImageResIcon(-1008);
}


BOOL DisplayOffCommand::Execute(const Parameter& param)
{
	if (mIsConfirmBeforeRun) {
		if (AfxMessageBox(_T("モニタの電源をオフにしますか?"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES) {
			return TRUE;
		}
	}

	PostMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);

	return TRUE;
}

launcherapp::core::Command* DisplayOffCommand::Clone()
{
	return new DisplayOffCommand(*this);
}


}
}
}
