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

struct ControlPanelCommand::PImpl
{
	CString mIconPath;
	CString mAppName;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(ControlPanelCommand)

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
	return _T("⏎:開く");
}

CString ControlPanelCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

BOOL ControlPanelCommand::Execute(Parameter* param)
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

// メニューの項目数を取得する
int ControlPanelCommand::GetMenuItemCount()
{
	return 1;
}

// メニューの表示名を取得する
bool ControlPanelCommand::GetMenuItemName(int index, LPCWSTR* displayNamePtr)
{
	if (index == 0) {
		static LPCWSTR name = L"開く(&O)";
		*displayNamePtr= name;
		return true;
	}
	return false;
}

// メニュー選択時の処理を実行する
bool ControlPanelCommand::SelectMenuItem(int index, launcherapp::core::CommandParameter* param)
{
	if (index == 0) {
		return Execute(param) != FALSE;
	}
	return false;
}

bool ControlPanelCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_CONTEXTMENUSOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ContextMenuSource*)this;
		return true;
	}
	return false;
}

CString ControlPanelCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_CONTROLPANEL);
	return TEXT_TYPE;
}


} // end of namespace controlpanel
} // end of namespace commands
} // end of namespace launcherapp

