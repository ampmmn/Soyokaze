#include "pch.h"
#include "hotkey/ExtraActionHotKeyHandler.h"
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


ExtraActionHotKeyHandler::ExtraActionHotKeyHandler(
	CString displayName,
 	launcherapp::core::Command* cmd,
 	const HOTKEY_ATTR& hotkeyAttr
) : mDisplayName(displayName), mCmd(cmd), mHotkeyAttr(hotkeyAttr)
{
	if (mCmd) {
		mCmd->AddRef();
	}
}

ExtraActionHotKeyHandler::~ExtraActionHotKeyHandler()
{
	if (mCmd) {
		mCmd->Release();
	}
}


CString ExtraActionHotKeyHandler::GetDisplayName()
{
	return mDisplayName;
}

bool ExtraActionHotKeyHandler::Invoke()
{
	if (mCmd == nullptr) {
		return false;
	}

	// 入力欄を非表示にして、コマンドを実行する。
	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	mainWnd->HideWindow();

	RefPtr<Action> action;
	if (mCmd->GetAction(mHotkeyAttr, &action) == false) {
		spdlog::error(_T("Failed to get action {}"), (LPCTSTR)mDisplayName);
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

bool ExtraActionHotKeyHandler::IsTemporaryHandler()
{
	return true;
}


uint32_t ExtraActionHotKeyHandler::AddRef()
{
	return (uint32_t)InterlockedIncrement(&mRefCount);
}

uint32_t ExtraActionHotKeyHandler::Release()
{
	auto n = InterlockedDecrement(&mRefCount);
	if (n == 0) {
		delete this;
	}
	return (uint32_t)n;
}

