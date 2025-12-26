#include "pch.h"
#include "CommandActionHandlerRegistry.h"
#include "mainwindow/controller/MainWindowController.h"
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/ExtraActionHotKeyHandler.h"
#include "actions/core/Action.h"
#include "actions/core/ActionParameter.h"
#include "core/IFIDDefine.h"
#include "commands/core/ExtraActionHotKeySettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using Command = launcherapp::core::Command;
using CommandHotKeyManager = launcherapp::core::CommandHotKeyManager;
using ExtraActionHotKeySettings = launcherapp::commands::core::ExtraActionHotKeySettings;

using namespace launcherapp::actions::core;

namespace launcherapp { namespace mainwindow {

struct CommandActionHandlerRegistry::PImpl
{
	void Update(CandidateList* candidateList) {
		auto cmd = candidateList->GetCurrentCommand();
		if (cmd == mCmd.get()) {
			// 変化がないため何もしない
			return;
		}

		Clear();
		Register(cmd);
	}

	void Clear() {
		auto manager = CommandHotKeyManager::GetInstance();
		for (auto handler : mHandlers) {
			manager->Unregister(handler);
			handler->Release();
		}
		mHandlers.clear();
		mCmd.reset();
	}

	void Register(Command* cmd) {

		if (cmd == nullptr) {
			return;
		}

		// 追加のアクション情報を持つコマンドかどうか?
		RefPtr<ExtraActionHotKeySettings> extraActionSettings;
		if (cmd->QueryInterface(IFID_EXTRAACTIONHOTKEYSETTINGS, (void**)&extraActionSettings) == false) {
			// 追加のアクションを持たない
			mCmd.reset();
			return;
		}

		// 追加のアクションを呼ぶためのホットキー情報をホットキーマネージャに登録しておく
		auto manager = CommandHotKeyManager::GetInstance();

		int count = extraActionSettings->GetHotKeyCount();
		for (int i = 0; i < count; ++i) {
			HOTKEY_ATTR hotkeyAttr;
			if (extraActionSettings->GetHotKeyAttribute(i, hotkeyAttr) == false) {
				continue;
			}

			CString handlerName;
			handlerName.Format(_T("_handler_%d"), i);
			auto handler = new ExtraActionHotKeyHandler(handlerName, cmd, hotkeyAttr);
			mHandlers.push_back(handler);

			manager->Register(this, handler, CommandHotKeyAttribute(hotkeyAttr.GetModifiers(), hotkeyAttr.GetVKCode()));
		}

		cmd->AddRef();
		mCmd.reset(cmd);
	}

	RefPtr<Command> mCmd;

	std::vector<ExtraActionHotKeyHandler*> mHandlers;
};

CommandActionHandlerRegistry::CommandActionHandlerRegistry() : in(new PImpl)
{
}

CommandActionHandlerRegistry::~CommandActionHandlerRegistry()
{
}

void CommandActionHandlerRegistry::Initialize(CandidateList* candidateList)
{
	candidateList->AddListener(this);
}

void CommandActionHandlerRegistry::Finalize(CandidateList* candidateList)
{
	candidateList->RemoveListener(this);
	in->Clear();
}

void CommandActionHandlerRegistry::OnUpdateSelect(void* sender)
{
	in->Update((CandidateList*)sender);
}

void CommandActionHandlerRegistry::OnUpdateItems(void* sender)
{
	in->Update((CandidateList*)sender);
}

}}

