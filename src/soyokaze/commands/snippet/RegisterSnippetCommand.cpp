#include "pch.h"
#include "framework.h"
#include "RegisterSnippetCommand.h"
#include "commands/snippet/SnippetCommand.h"
#include "commands/core/CommandRepository.h"
#include "mainwindow/controller/MainWindowController.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace snippet {

using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;


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

BOOL RegisterSnippetCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// ウインドウ経由でクリップボードのテキストを取得
	CString clipboardText;
	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	mainWnd->GetClipboardString(clipboardText);

	RefPtr<CommandParameterBuilder> inParam(CommandParameterBuilder::Create(), false);
	inParam->SetNamedParamString(_T("TEXT"), clipboardText);
	SnippetCommand::NewDialog(inParam);

	// キャンセルされてもエラーダイアログを出さないようにするため、常にTRUEをかえす
	return TRUE;
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

