#include "pch.h"
#include "framework.h"
#include "commands/builtin/AfxChangeDirectoryCommand.h"
#include "commands/common/AfxWWrapper.h"
#include "commands/core/CommandRepository.h"
#include "utility/ScopeAttachThreadInput.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

CString AfxChangeDirectoryCommand::TYPE(_T("Builtin-AfxCD"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(AfxChangeDirectoryCommand)

CString AfxChangeDirectoryCommand::GetType()
{
	return TYPE;
}

AfxChangeDirectoryCommand::AfxChangeDirectoryCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("afxcd"))
{
	mDescription = _T("【afxwカレントディレクトリ変更】");
}

AfxChangeDirectoryCommand::~AfxChangeDirectoryCommand()
{
}

BOOL AfxChangeDirectoryCommand::Execute(const Parameter& param)
{
	CString path = param.GetWholeString();
	if (PathFileExists(path) == FALSE) {
		return TRUE;
	}

	// あふw上のカレントディレクトリ変更
	AfxWWrapper afxw;
	afxw.SetCurrentDir(path);

	// あふwをアクティブにする
	HWND hwndApp = FindWindow(_T("TAfxWForm"), nullptr);
	if (IsWindow(hwndApp)) {
		ScopeAttachThreadInput scope;

		LONG_PTR style = GetWindowLongPtr(hwndApp, GWL_STYLE);
		if ((style & WS_MAXIMIZE) == 0) {
			PostMessage(hwndApp, WM_SYSCOMMAND, SC_RESTORE, 0);
		}
		SetForegroundWindow(hwndApp);
	}

	return TRUE;
}

HICON AfxChangeDirectoryCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-185);
}

launcherapp::core::Command* AfxChangeDirectoryCommand::Clone()
{
	return new AfxChangeDirectoryCommand();
}

launcherapp::core::Command* AfxChangeDirectoryCommand::Create(LPCTSTR name)
{
	return new AfxChangeDirectoryCommand(name);
}

}
}
}
