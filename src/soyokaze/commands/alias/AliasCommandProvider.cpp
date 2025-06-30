#include "pch.h"
#include "AliasCommandProvider.h"
#include "commands/alias/AliasCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "commands/core/CommandFile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace alias {


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(AliasCommandProvider)

IMPLEMENT_LOADFROM(AliasCommandProvider, AliasCommand)

AliasCommandProvider::AliasCommandProvider()
{
}

AliasCommandProvider::~AliasCommandProvider()
{
}

CString AliasCommandProvider::GetName()
{
	return _T("AliasCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString AliasCommandProvider::GetDisplayName()
{
	return _T("エイリアスコマンド");
}

// コマンドの種類の説明を示す文字列を取得
CString AliasCommandProvider::GetDescription()
{
	return _T("コマンドに対する別名を設定します。\n")
		     _T("登録型でないコマンドに対して任意の名前を設定したり\n")
		     _T("ホットキーを割り当てたりすることができます");
}

// コマンド新規作成ダイアログ
bool AliasCommandProvider::NewDialog(CommandParameter* param)
{
	return AliasCommand::NewDialog(param);
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t AliasCommandProvider::AliasCommandProvider::GetOrder() const
{
	return 5100;
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t AliasCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(AliasCommand::TypeDisplayName());
	return 1;
}

}
}
}
