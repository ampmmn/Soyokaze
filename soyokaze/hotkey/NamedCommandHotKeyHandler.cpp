#include "pch.h"
#include "hotkey/NamedCommandHotKeyHandler.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"

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
	CWnd* wnd = AfxGetMainWnd();
	ASSERT(wnd);
	ShowWindow(wnd->GetSafeHwnd(), SW_HIDE);

	launcherapp::core::CommandParameter param;
	param.SetNamedParamBool(_T("OnHotKey"), _T("true"));
	bool result = cmd->Execute(param);
	cmd->Release();

	return result;
}

