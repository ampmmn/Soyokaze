#include "pch.h"
#include "framework.h"
#include "PathExecuteCommand.h"
#include "commands/pathfind/ExcludePathList.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/core/ActionParameter.h"
#include "actions/builtin/ExecuteAction.h"
#include "actions/builtin/OpenPathInFilerAction.h"
#include "actions/builtin/ShowPropertiesAction.h"
#include "actions/builtin/ShowPropertiesAction.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "utility/Path.h"
#include "setting/AppPreference.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "mainwindow/controller/MainWindowController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using ExecuteAction = launcherapp::actions::builtin::ExecuteAction;
using ShowPropertiesAction = launcherapp::actions::builtin::ShowPropertiesAction;
using OpenPathInFilerAction = launcherapp::actions::builtin::OpenPathInFilerAction;
using CopyTextAction = launcherapp::actions::clipboard::CopyTextAction;

namespace launcherapp {
namespace commands {
namespace pathfind {

struct PathExecuteCommand::PImpl
{
	CString mWord;
	CString mFullPath;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(PathExecuteCommand)

PathExecuteCommand::PathExecuteCommand(const CString& fullPath) : in(std::make_unique<PImpl>())
{
	in->mWord = fullPath;
	in->mFullPath = fullPath;
	this->mDescription = fullPath;
}

PathExecuteCommand::PathExecuteCommand(const CString& name, const CString& fullPath) : in(std::make_unique<PImpl>())
{
	in->mWord = name;
	in->mFullPath = fullPath;
	this->mDescription = fullPath;
}

PathExecuteCommand::~PathExecuteCommand()
{
}

CString PathExecuteCommand::GetName()
{
	if (in->mFullPath.IsEmpty()) {
		return _T("");
	}
	return PathFindFileName(in->mFullPath);
}

CString PathExecuteCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool PathExecuteCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (Path::FileExists(in->mFullPath) == FALSE) {
		// 存在しないパスの場合は何も実行しない
		return false;
	}

	if (modifierFlags == 0) {
		// 実行
		auto a = new ExecuteAction(in->mFullPath, _T("$*"));
		a->SetHistoryPolicy(ExecuteAction::HISTORY_ALWAYS);
		*action = a;
		return true;
	}

	if (modifierFlags == (Command::MODIFIER_SHIFT | Command::MODIFIER_CTRL)) {
		// 管理者権限で実行
		auto a = new ExecuteAction(in->mFullPath, _T("$*"));
		a->SetHistoryPolicy(ExecuteAction::HISTORY_ALWAYS);
		a->SetRunAsAdmin();
		*action = a;
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_CTRL) {
		// パスを開く
		*action = new OpenPathInFilerAction(in->mFullPath);
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_ALT) {
		// プロパティ表示
		*action = new ShowPropertiesAction(in->mFullPath);
		return true;
	}
	return false;
}

HICON PathExecuteCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mFullPath);
}

launcherapp::core::Command*
PathExecuteCommand::Clone()
{
	auto clonedObj = make_refptr<PathExecuteCommand>(in->mWord, in->mFullPath);
	return clonedObj.release();
}

// メニューの項目数を取得する
int PathExecuteCommand::GetMenuItemCount()
{
	return 5;
}

// メニューの表示名を取得する
bool PathExecuteCommand::GetMenuItem(int index, Action** action)
{
	if (index < 0 || 4 < index) {
		return false;
	}

	if (index == 0) {
		return GetAction(0, action);
	}
	else if (index == 1) {
		*action = new OpenPathInFilerAction(in->mFullPath);
		return true;
	}
	else if (index == 2)  {
		// 管理者権限で実行
		auto a = new ExecuteAction(in->mFullPath);
		a->SetHistoryPolicy(ExecuteAction::HISTORY_ALWAYS);
		a->SetRunAsAdmin();
		*action = a;
		return true;
	}
	else if (index == 3) {
		// クリップボードにコピー
		*action = new CopyTextAction(in->mFullPath);
		return true;
	}
	else { // if (index == 4)
				 // プロパティダイアログを表示
		*action = new ShowPropertiesAction(in->mFullPath);
		return true;
	}
}

bool PathExecuteCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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

CString PathExecuteCommand::TypeDisplayName()
{
	static CString TEXT_TYPE_ADHOC((LPCTSTR)IDS_COMMAND_PATHEXEC);
	return TEXT_TYPE_ADHOC;
}


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace launcherapp

