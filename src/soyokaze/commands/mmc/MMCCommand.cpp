#include "pch.h"
#include "framework.h"
#include "MMCCommand.h"
#include "actions/builtin/ExecuteAction.h"
#include "utility/Path.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using ExecuteAction = launcherapp::actions::builtin::ExecuteAction;

namespace launcherapp {
namespace commands {
namespace mmc {

struct MMCCommand::PImpl
{
	MMCSnapin mSnapin;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(MMCCommand)

MMCCommand::MMCCommand(const MMCSnapin& snapin) : 
	AdhocCommandBase(snapin.mDisplayName, snapin.mDescription),
	in(std::make_unique<PImpl>())
{
	in->mSnapin = snapin;
}

MMCCommand::~MMCCommand()
{
}

CString MMCCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool MMCCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (hotkeyAttr.GetModifiers() != 0) {
		return false;
	}

	Path cmdExePath(Path::SYSTEMDIR, _T("cmd.exe"));

	CString paramStr;
	paramStr.Format(_T("/c start \"\" \"%s\""), (LPCTSTR)in->mSnapin.mMscFilePath);

	*action = new ExecuteAction((LPCTSTR)cmdExePath, paramStr, _T(""), SW_HIDE);
	return true;
}

HICON MMCCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconResource(in->mSnapin.mIconFilePath, in->mSnapin.mIconIndex);
}

launcherapp::core::Command*
MMCCommand::Clone()
{
	return new MMCCommand(in->mSnapin);
}

// メニューの項目数を取得する
int MMCCommand::GetMenuItemCount()
{
	return 1;
}

// メニューの表示名を取得する
bool MMCCommand::GetMenuItem(int index, Action** action)
{
	if (index == 0) {
		return GetAction(HOTKEY_ATTR(0, VK_RETURN), action);
	}
	return false;
}

bool MMCCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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

CString MMCCommand::TypeDisplayName()
{
	static CString TEXT_TYPE(_T("MMCスナップイン"));
	return TEXT_TYPE;
}


} // end of namespace mmc
} // end of namespace commands
} // end of namespace launcherapp

