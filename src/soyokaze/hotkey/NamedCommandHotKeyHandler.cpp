#include "pch.h"
#include "hotkey/NamedCommandHotKeyHandler.h"
#include "commands/core/CommandRepository.h"
#include "commands/common/Message.h"
#include "actions/core/Action.h"
#include "actions/core/ActionParameter.h"
#include "mainwindow/controller/MainWindowController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::core;
using namespace launcherapp::actions::core;

NamedCommandHotKeyHandler::NamedCommandHotKeyHandler(
	CString name
) : mName(name)
{
}

NamedCommandHotKeyHandler::~NamedCommandHotKeyHandler()
{
}


CString NamedCommandHotKeyHandler::GetDisplayName()
{
	return mName;
}

bool NamedCommandHotKeyHandler::Invoke()
{
	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();

	RefPtr<Command> cmd(cmdRepoPtr->QueryAsWholeMatch(mName, false));
	if (cmd.get() == nullptr) {
		return false;
	}

	// 入力欄を非表示にして、コマンドを実行する。
	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	mainWnd->HideWindow();

	RefPtr<Action> action;
	if (cmd->GetAction(0, &action) == false) {
		spdlog::error(_T("Failed to get action {}"), (LPCTSTR)mName);
		return false;
	}

	RefPtr<ParameterBuilder> param(ParameterBuilder::Create());
	param->SetNamedParamBool(_T("OnHotKey"), _T("true"));
	String errMsg;
	if (action->Perform(param.get(), &errMsg) == false) {
		CString tmp;
		launcherapp::commands::common::PopupMessage(UTF2UTF(errMsg, tmp));
		return false;
	}
	return true;
}

