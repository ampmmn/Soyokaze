#include "pch.h"
#include "framework.h"
#include "RegisterSnippetCommand.h"
#include "commands/snippet/SnippetCommand.h"
#include "core/CommandRepository.h"
#include "SharedHwnd.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace snippet {

CString RegisterSnippetCommand::TYPE(_T("Builtin-RegisterSnippet"));
CString RegisterSnippetCommand::DEFAULT_NAME(_T("newsnippet"));

CString RegisterSnippetCommand::GetType()
{
	return TYPE;
}

RegisterSnippetCommand::RegisterSnippetCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : DEFAULT_NAME)
{
	mDescription = _T("【クリップボードのテキストを定型文として登録】");
}

RegisterSnippetCommand::~RegisterSnippetCommand()
{
}

BOOL RegisterSnippetCommand::Execute(const Parameter& param)
{
	// ウインドウ経由でクリップボードのテキストを取得
	CString clipboardText;
	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 10, 0, (LPARAM)&clipboardText);

	Parameter inParam;
	inParam.SetNamedParamString(_T("TEXT"), clipboardText);
	SnippetCommand::NewDialog(&inParam);

	// キャンセルされてもエラーダイアログを出さないようにするため、常にTRUEをかえす
	return TRUE;
}

HICON RegisterSnippetCommand::GetIcon()
{
	return IconLoader::Get()->LoadNewIcon();
}

soyokaze::core::Command* RegisterSnippetCommand::Clone()
{
	return new RegisterSnippetCommand();
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

