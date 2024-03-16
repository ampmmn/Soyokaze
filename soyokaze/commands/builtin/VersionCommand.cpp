#include "pch.h"
#include "VersionCommand.h"
#include "gui/AboutDlg.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

CString VersionCommand::TYPE(_T("Builtin-Version"));

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

VersionCommand::~VersionCommand()
{
}

BOOL VersionCommand::Execute(const Parameter& param)
{
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

soyokaze::core::Command*
VersionCommand::Clone()
{
	return new VersionCommand();
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

