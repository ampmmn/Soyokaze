#include "pch.h"
#include "framework.h"
#include "commands/builtin/AfxChangeDirectoryCommand.h"
#include "commands/share/AfxWWrapper.h"
#include "commands/core/CommandRepository.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/Path.h"
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
	mCanSetConfirm = false;
	mCanDisable = true;
	mIsEnable = false;
}

AfxChangeDirectoryCommand::AfxChangeDirectoryCommand(const AfxChangeDirectoryCommand& rhs) :
	BuiltinCommandBase(rhs)
{
}

AfxChangeDirectoryCommand::~AfxChangeDirectoryCommand()
{
}

BOOL AfxChangeDirectoryCommand::Execute(Parameter* param)
{
	CString path = param->GetWholeString();
	if (Path::FileExists(path) == FALSE) {
		return TRUE;
	}

	// あふw上のカレントディレクトリ変更
	AfxWWrapper afxw;
	afxw.SetCurrentDir((LPCTSTR)path);

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
	return new AfxChangeDirectoryCommand(*this);
}

}
}
}
