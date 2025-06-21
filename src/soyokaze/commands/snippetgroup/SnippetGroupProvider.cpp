#include "pch.h"
#include "SnippetGroupProvider.h"
#include "commands/snippetgroup/SnippetGroupCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "commands/core/CommandFile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace snippetgroup {

using CommandRepository = launcherapp::core::CommandRepository;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(SnippetGroupProvider)

SnippetGroupProvider::SnippetGroupProvider()
{
}

SnippetGroupProvider::~SnippetGroupProvider()
{
}

CString SnippetGroupProvider::GetName()
{
	return _T("SnippetGroupCommandProvider");
}

// 作成できるコマンドの種類を表す文字列を取得
CString SnippetGroupProvider::GetDisplayName()
{
	return CString((LPCTSTR)_T("定型文グループコマンド"));
}

// コマンドの種類の説明を示す文字列を取得
CString SnippetGroupProvider::GetDescription()
{
	CString description;
	description += _T("複数の定型文を登録をするためのグループを定義します");
	return description;
}

// コマンド新規作成ダイアログ
bool SnippetGroupProvider::NewDialog(CommandParameter* param)
{
	SnippetGroupCommand* newCmd = nullptr;
	if (SnippetGroupCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}

	CommandRepository::GetInstance()->RegisterCommand(newCmd);
	return true;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t SnippetGroupProvider::GetOrder() const
{
	return 140;
}

void SnippetGroupProvider::OnBeforeLoad()
{
}

bool SnippetGroupProvider::LoadFrom(CommandEntryIF* entry, Command** retCommand)
{
	std::unique_ptr<SnippetGroupCommand> command(new SnippetGroupCommand);
	if (command->Load(entry) == false) {
		return false;
	}
	ASSERT(retCommand);

	*retCommand = command.release();
	return true;
}



} // end of namespace snippetgroup
} // end of namespace commands
} // end of namespace launcherapp

