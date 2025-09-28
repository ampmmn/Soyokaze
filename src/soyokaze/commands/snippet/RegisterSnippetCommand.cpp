#include "pch.h"
#include "framework.h"
#include "RegisterSnippetCommand.h"
#include "commands/snippet/SnippetCommand.h"
#include "commands/core/CommandRepository.h"
#include "actions/core/ActionParameter.h"
#include "actions/builtin/CallbackAction.h"
#include "mainwindow/controller/MainWindowController.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace snippet {

using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;
using CallbackAction = launcherapp::actions::builtin::CallbackAction;

CString RegisterSnippetCommand::TYPE(_T("Builtin-RegisterSnippet"));
CString RegisterSnippetCommand::DEFAULT_NAME(_T("newsnippet"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(RegisterSnippetCommand)

CString RegisterSnippetCommand::GetType()
{
	return TYPE;
}

RegisterSnippetCommand::RegisterSnippetCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : DEFAULT_NAME)
{
	mDescription = _T("【クリップボードのテキストを定型文として登録】");
	mCanSetConfirm = false;
	mCanDisable = true;
}

RegisterSnippetCommand::~RegisterSnippetCommand()
{
}

bool RegisterSnippetCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags != 0) {
		return false;
	}

	*action = new CallbackAction(_T("開く"), [&](Parameter*, String*) -> bool {
		// ウインドウ経由でクリップボードのテキストを取得
		CString clipboardText;
		auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
		mainWnd->GetClipboardString(clipboardText);
	
		RefPtr<ParameterBuilder> inParam(ParameterBuilder::Create(), false);
		inParam->SetNamedParamString(_T("TEXT"), clipboardText);
		SnippetCommand::NewDialog(inParam);
	
		// キャンセルされてもエラーダイアログを出さないようにするため、常にTRUEをかえす
		return true;
	});
	return true;
}

HICON RegisterSnippetCommand::GetIcon()
{
	return IconLoader::Get()->LoadNewIcon();
}

launcherapp::core::Command* RegisterSnippetCommand::Clone()
{
	return new RegisterSnippetCommand();
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

