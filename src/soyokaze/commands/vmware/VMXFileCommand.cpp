#include "pch.h"
#include "framework.h"
#include "VMXFileCommand.h"
#include "core/IFIDDefine.h"
#include "actions/vmware/RunVMXAction.h"
#include "actions/builtin/OpenPathInFilerAction.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using RunVMXAction = launcherapp::actions::vmware::RunVMXAction;
using OpenPathInFilerAction = launcherapp::actions::builtin::OpenPathInFilerAction;

namespace launcherapp {
namespace commands {
namespace vmware {

struct VMXFileCommand::PImpl
{
	CString mName;
	CString mFullPath;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(VMXFileCommand)


VMXFileCommand::VMXFileCommand(const CString& name, const CString& fullPath) : in(std::make_unique<PImpl>())
{
	in->mName = name;
	this->mName = name;
	this->mDescription = fullPath;
	in->mFullPath = fullPath;
}

VMXFileCommand::~VMXFileCommand()
{
}

CString VMXFileCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool VMXFileCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == 0) {
		// VMを実行する
		*action = new RunVMXAction(in->mName, in->mFullPath, SW_SHOW);
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_CTRL) {
		// パスを開く
		*action = new OpenPathInFilerAction(in->mFullPath);
		return true;
	}
	return false;
}

HICON VMXFileCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mFullPath);
}

launcherapp::core::Command*
VMXFileCommand::Clone()
{
	return new VMXFileCommand(this->mName, in->mFullPath);
}

// メニューの項目数を取得する
int VMXFileCommand::GetMenuItemCount()
{
	return 3;
}

// メニューの表示名を取得する
bool VMXFileCommand::GetMenuItem(int index, Action** action)
{
	if (index < 0 || 2 < index) {
		return false;
	}

	if (index == 0) {
		*action = new RunVMXAction(in->mName, in->mFullPath, SW_SHOW);
		return true;
	}
	else if (index == 1) {
		*action = new RunVMXAction(in->mName, in->mFullPath, SW_MAXIMIZE);
		return true;
	}
	else {
		*action = new RunVMXAction(in->mName, in->mFullPath, SW_MINIMIZE);
		return true;
	}
}

bool VMXFileCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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

CString VMXFileCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_VMXFILE);
	return TEXT_TYPE;
}


} // end of namespace vmware
} // end of namespace commands
} // end of namespace launcherapp

