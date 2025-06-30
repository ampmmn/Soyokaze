#include "pch.h"
#include "SnippetCommandProvider.h"
#include "commands/snippet/SnippetCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "commands/core/CommandFile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace snippet {


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(SnippetCommandProvider)

IMPLEMENT_LOADFROM(SnippetCommandProvider, SnippetCommand)

SnippetCommandProvider::SnippetCommandProvider()
{
}

SnippetCommandProvider::~SnippetCommandProvider()
{
}

CString SnippetCommandProvider::GetName()
{
	return _T("SnippetCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString SnippetCommandProvider::GetDisplayName()
{
	return CString((LPCTSTR)IDS_SNIPPETCOMMAND);
}

// コマンドの種類の説明を示す文字列を取得
CString SnippetCommandProvider::GetDescription()
{
	return CString((LPCTSTR)IDS_DESCRIPTION_SNIPPETCOMMAND);
}

// コマンド新規作成ダイアログ
bool SnippetCommandProvider::NewDialog(CommandParameter* param)
{
	return SnippetCommand::NewDialog(param);
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t SnippetCommandProvider::SnippetCommandProvider::GetOrder() const
{
	return 120;
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t SnippetCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(SnippetCommand::TypeDisplayName());
	return 1;
}

}
}
}
