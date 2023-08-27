#include "pch.h"
#include "NamedCommandHotKeyHandler.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"

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
	CWnd* wnd = AfxGetMainWnd();
	ASSERT(wnd);
	ShowWindow(wnd->GetSafeHwnd(), SW_HIDE);

	soyokaze::core::CommandParameter param;
	bool result = cmd->Execute(param);
	cmd->Release();

	return result;
}

