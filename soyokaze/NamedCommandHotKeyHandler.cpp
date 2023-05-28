#include "pch.h"
#include "NamedCommandHotKeyHandler.h"
#include "CommandRepository.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

NamedCommandHotKeyHandler::NamedCommandHotKeyHandler(
	CommandRepository* cmdReposPtr,
	CString name
) : mCmdReposPtr(cmdReposPtr), mName(name)
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
	auto cmd = mCmdReposPtr->QueryAsWholeMatch(mName, false);
	if (cmd == nullptr) {
		return false;
	}

	// 入力欄を非表示にして、コマンドを実行する。
	ShowWindow(AfxGetMainWnd()->GetSafeHwnd(), SW_HIDE);

	return cmd->Execute();
}

