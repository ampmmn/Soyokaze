#include "pch.h"
#include "framework.h"
#include "ControlPanelCommand.h"
#include "commands/common/SubProcess.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace controlpanel {

constexpr LPCTSTR TYPENAME = _T("ControlPanelCommand");

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
	in(std::make_unique<PImpl>())
{
	in->mIconPath = iconPath;
	in->mAppName = appName;
}

ControlPanelCommand::~ControlPanelCommand()
{
}

CString ControlPanelCommand::GetGuideString()
{
	return _T("Enter:開く");
}

/**
 * 種別を表す文字列を取得する
 * @return 文字列
 */
CString ControlPanelCommand::GetTypeName()
{
	return TYPENAME;
}

CString ControlPanelCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_CONTROLPANEL);
	return TEXT_TYPE;
}

BOOL ControlPanelCommand::Execute(const Parameter& param)
{
	SubProcess exec(param);
	SubProcess::ProcessPtr process;
	return exec.Run(_T("control.exe"), _T("/name ") + in->mAppName, process);
}

HICON ControlPanelCommand::GetIcon()
{
	if (in->mIconPath.IsEmpty()) {
		return IconLoader::Get()->LoadUnknownIcon();
	}
	return IconLoader::Get()->GetDefaultIcon(in->mIconPath);
}

launcherapp::core::Command*
ControlPanelCommand::Clone()
{
	return new ControlPanelCommand(this->mName, in->mIconPath, in->mAppName, this->mDescription);
}

} // end of namespace controlpanel
} // end of namespace commands
} // end of namespace launcherapp

