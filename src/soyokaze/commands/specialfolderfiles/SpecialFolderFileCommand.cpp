#include "pch.h"
#include "framework.h"
#include "SpecialFolderFileCommand.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/core/ActionParameter.h"
#include "actions/builtin/ExecuteAction.h"
#include "actions/builtin/OpenPathInFilerAction.h"
#include "actions/builtin/CallbackAction.h"
#include "actions/builtin/NullAction.h"

#include "icon/IconLoader.h"
#include "setting/AppPreference.h"
#include "utility/Path.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using NamedParameter = launcherapp::actions::core::NamedParameter;
using ExecuteAction = launcherapp::actions::builtin::ExecuteAction;
using OpenPathInFilerAction = launcherapp::actions::builtin::OpenPathInFilerAction;
using CallbackAction = launcherapp::actions::builtin::CallbackAction;
using NullAction = launcherapp::actions::builtin::NullAction;

namespace launcherapp {
namespace commands {
namespace specialfolderfiles {


struct SpecialFolderFileCommand::PImpl
{
	ITEM mItem;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(SpecialFolderFileCommand)

SpecialFolderFileCommand::SpecialFolderFileCommand(const ITEM& item) : 
	AdhocCommandBase(item.mName, item.mDescription),
	in(std::make_unique<PImpl>())
{
	in->mItem = item;
	if (item.mDescription.IsEmpty()) {
		this->mDescription = item.mFullPath;
	}
}

SpecialFolderFileCommand::~SpecialFolderFileCommand()
{
}

CString SpecialFolderFileCommand::GetTypeDisplayName()
{
	return TypeDisplayName((int)in->mItem.mType);
}

bool SpecialFolderFileCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == 0) {
		*action = new ExecuteAction(in->mItem.mFullPath);
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_CTRL) {
		*action = new OpenPathInFilerAction(in->mItem.mFullPath);
		return true;
	}
	return false;
}

HICON SpecialFolderFileCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mItem.mFullPath);
}

launcherapp::core::Command*
SpecialFolderFileCommand::Clone()
{
	return new SpecialFolderFileCommand(in->mItem);
}

// メニューの項目数を取得する
int SpecialFolderFileCommand::GetMenuItemCount()
{
	return in->mItem.mType == TYPE_RECENT ? 3 : 2;
}

// メニューの表示名を取得する
bool SpecialFolderFileCommand::GetMenuItem(int index, Action** action)
{
	if (index < 0 || 2 < index) {
		return false;
	}

	if (index == 0) {
		*action = new ExecuteAction(in->mItem.mFullPath);
		return true;
	}

	if (index == 1) {
		*action = new OpenPathInFilerAction(in->mItem.mFullPath);
		return true;
	}
	else  {
		// 削除
		if (in->mItem.mType != TYPE_RECENT) {
			// 削除できるのは履歴のみ
			*action = new NullAction();
			return true;
		}
		
		*action = new CallbackAction(_T("最近使ったファイルから削除する"), [&](Parameter*, String*) -> bool {

			if (Path::FileExists(in->mItem.mLinkPath) == FALSE) {
				return true;
			}
			// ショートカットを削除
			DeleteFile(in->mItem.mLinkPath);
			return true;
		});
		return true;
	}
}

bool SpecialFolderFileCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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

CString SpecialFolderFileCommand::TypeDisplayName(int type)
{
	if (type == TYPE_RECENT) {
		static CString TEXT_TYPE_RECENT((LPCTSTR)IDS_COMMAND_RECENTFILES);
		return TEXT_TYPE_RECENT;
	}
	else { // if (type == TYPE_STARTMENU)
		static CString TEXT_TYPE_STARTMENU((LPCTSTR)IDS_COMMAND_STARTMENU);
		return TEXT_TYPE_STARTMENU;
	}
}

} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace launcherapp

