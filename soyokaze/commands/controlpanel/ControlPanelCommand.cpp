#include "pch.h"
#include "framework.h"
#include "ControlPanelCommand.h"
#include "IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace controlpanel {


struct ControlPanelCommand::PImpl
{
	CString mIconPath;
	CString mAppName;
};


ControlPanelCommand::ControlPanelCommand(
	const CString& name,
	const CString& iconPath,
	const CString& appName,
	const CString& description
) : AdhocCommandBase(name, description),
	in(new PImpl)
{
	in->mIconPath = iconPath;
	in->mAppName = appName;
}

ControlPanelCommand::~ControlPanelCommand()
{
}

CString ControlPanelCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_CONTROLPANEL);
	return TEXT_TYPE;
}

BOOL ControlPanelCommand::Execute(const Parameter& param)
{
	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_NORMAL;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = _T("control.exe");

	CString paramStr(_T("/name ") + in->mAppName);
	
	si.lpParameters = paramStr;

	ShellExecuteEx(&si);
	CloseHandle(si.hProcess);

	return TRUE;
}

HICON ControlPanelCommand::GetIcon()
{
	if (in->mIconPath.IsEmpty()) {
		return IconLoader::Get()->LoadUnknownIcon();
	}
	return IconLoader::Get()->GetDefaultIcon(in->mIconPath);
}

soyokaze::core::Command*
ControlPanelCommand::Clone()
{
	return new ControlPanelCommand(this->mName, in->mIconPath, in->mAppName, this->mDescription);
}

} // end of namespace controlpanel
} // end of namespace commands
} // end of namespace soyokaze

