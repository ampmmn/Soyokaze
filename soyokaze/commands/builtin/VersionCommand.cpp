#include "pch.h"
#include "VersionCommand.h"
#include "commands/builtin/AboutDlg.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

CString VersionCommand::TYPE(_T("Builtin-Version"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(VersionCommand)

CString VersionCommand::GetType()
{
	return TYPE;
}

VersionCommand::VersionCommand(LPCTSTR name) : 
	BuiltinCommandBase(name ? name : _T("version")),
	mIsExecuting(false)
{
	mDescription = _T("【バージョン情報】");
}

VersionCommand::VersionCommand(const VersionCommand& rhs) :
	BuiltinCommandBase(rhs),
	mIsExecuting(rhs.mIsExecuting)
{
}

VersionCommand::~VersionCommand()
{
}

BOOL VersionCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	if (mIsExecuting) {
		return TRUE;
	}

	CAboutDlg dlg;

	mIsExecuting = true;
	dlg.DoModal();
	mIsExecuting = false;
	return TRUE;
}

HICON VersionCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-81);
}

launcherapp::core::Command*
VersionCommand::Clone()
{
	return new VersionCommand(*this);
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

