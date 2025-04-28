#include "pch.h"
#include "hotkey/NamedCommandHotKeyHandler.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "mainwindow/controller/MainWindowController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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
	auto cmd = cmdRepoPtr->QueryAsWholeMatch(mName, false);
	if (cmd == nullptr) {
		return false;
	}

	// 入力欄を非表示にして、コマンドを実行する。
	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	mainWnd->HideWindow();

	auto param = launcherapp::core::CommandParameterBuilder::Create();
	param->SetNamedParamBool(_T("OnHotKey"), _T("true"));
	bool result = cmd->Execute(param);

	param->Release();
	cmd->Release();

	return result;
}

