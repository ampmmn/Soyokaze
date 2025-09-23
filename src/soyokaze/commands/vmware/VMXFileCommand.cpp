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

CString VMXFileCommand::GetGuideString()
{
	return _T("⏎:ファイルを開く C-⏎:パスを開く");
}

CString VMXFileCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool VMXFileCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags & Command::MODIFIER_CTRL) {
		// パスを開く
		*action = new OpenPathInFilerAction(in->mFullPath);
		return true;
	}
	else {
		// VMを実行する
		*action = new RunVMXAction(in->mName, in->mFullPath, SW_SHOW);
		return true;
	}
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
bool VMXFileCommand::GetMenuItemName(int index, LPCWSTR* displayNamePtr)
{
	if (index == 0) {
		static LPCWSTR name = L"実行(&E)";
		*displayNamePtr= name;
		return true;
	}
	else if (index == 1) {
		static LPCWSTR name = L"最大化状態で実行(&X)";
		*displayNamePtr= name;
		return true;
	}
	else if (index == 2) {
		static LPCWSTR name = L"最小化状態で実行(&M)";
		*displayNamePtr= name;
		return true;
	}
	return false;
}

// メニュー選択時の処理を実行する
bool VMXFileCommand::SelectMenuItem(int index, Parameter* param)
{
	if (index < 0 || 2 < index) {
		return false;
	}

	if (index == 0) {
		RunVMXAction action(in->mName, in->mFullPath, SW_SHOW);
		return action.Perform(param, nullptr);
	}
	else if (index == 1) {
		RunVMXAction action(in->mName, in->mFullPath, SW_MAXIMIZE);
		return action.Perform(param, nullptr);
	}
	else {
		RunVMXAction action(in->mName, in->mFullPath, SW_MINIMIZE);
		return action.Perform(param, nullptr);
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

