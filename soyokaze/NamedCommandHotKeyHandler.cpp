#include "pch.h"
#include "NamedCommandHotKeyHandler.h"
#include "core/CommandRepository.h"

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
	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();
	auto cmd = cmdRepoPtr->QueryAsWholeMatch(mName, false);
	if (cmd == nullptr) {
		return false;
	}

	// 入力欄を非表示にして、コマンドを実行する。
	ShowWindow(AfxGetMainWnd()->GetSafeHwnd(), SW_HIDE);

	bool result = cmd->Execute();
	cmd->Release();

	return result;
}

