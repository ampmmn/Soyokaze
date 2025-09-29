#include "pch.h"
#include "framework.h"
#include "PathURLCommand.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/core/ActionParameter.h"
#include "actions/builtin/ExecuteAction.h"
#include "actions/builtin/CallbackAction.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include "mainwindow/controller/MainWindowController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using ExecuteAction = launcherapp::actions::builtin::ExecuteAction;
using CallbackAction = launcherapp::actions::builtin::CallbackAction;

namespace launcherapp {
namespace commands {
namespace pathfind {

struct PathURLCommand::PImpl
{
	CString mURL;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(PathURLCommand)

PathURLCommand::PathURLCommand(const CString& url) : in(std::make_unique<PImpl>())
{
	in->mURL = url;
	this->mDescription = url;
}

PathURLCommand::~PathURLCommand()
{
}

CString PathURLCommand::GetName()
{
	return in->mURL;
}

CString PathURLCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool PathURLCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == 0) {
		// 実行
		auto a = new ExecuteAction(in->mURL, _T("$*"));
		a->SetHistoryPolicy(ExecuteAction::HISTORY_ALWAYS);
		*action = a;
		return true;
	}
	return false;
}

HICON PathURLCommand::GetIcon()
{
	return IconLoader::Get()->LoadIconFromPath(in->mURL);
}

launcherapp::core::Command*
PathURLCommand::Clone()
{
	auto clonedObj = make_refptr<PathURLCommand>(in->mURL);
	return clonedObj.release();
}

// メニューの項目数を取得する
int PathURLCommand::GetMenuItemCount()
{
	return 2;
}

// メニューの表示名を取得する
bool PathURLCommand::GetMenuItem(int index, Action** action)
{
	if (index == 0) {
		return GetAction(0, action);
	}
	if (index == 1) {
		*action = new CallbackAction(_T("URLをコマンドとして登録する"), [&](Parameter*, String*) -> bool {
			// URLをコマンドとして登録

			// 登録用のコマンド文字列を生成
			CString cmdStr;
			cmdStr.Format(_T("new \"\" %s"), (LPCTSTR)in->mURL);

			auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
			bool isWaitSync = false;
			mainWnd->RunCommand((LPCTSTR)cmdStr, isWaitSync);
			return true;
		});
		return true;
	}
	return false;
}

bool PathURLCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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

CString PathURLCommand::TypeDisplayName()
{
	static CString TEXT_TYPE_ADHOC((LPCTSTR)IDS_COMMAND_PATHEXEC);
	return TEXT_TYPE_ADHOC;
}


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace launcherapp

