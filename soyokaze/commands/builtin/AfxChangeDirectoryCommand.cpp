#include "pch.h"
#include "framework.h"
#include "commands/builtin/AfxChangeDirectoryCommand.h"
#include "commands/common/AfxWWrapper.h"
#include "core/CommandRepository.h"
#include "utility/ScopeAttachThreadInput.h"
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

CString AfxChangeDirectoryCommand::TYPE(_T("Builtin-AfxCD"));

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
	std::vector<CString> args;
	param.GetParameters(args);

	if (PathFileExists(args[0]) == FALSE) {
		return TRUE;
	}

	// あふw上のカレントディレクトリ変更
	AfxWWrapper afxw;
	afxw.SetCurrentDir(args[0]);

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

soyokaze::core::Command* AfxChangeDirectoryCommand::Clone()
{
	return new AfxChangeDirectoryCommand();
}

}
}
}
