#include "pch.h"
#include "KeySplitterCommandProvider.h"
#include "commands/keysplitter/KeySplitterCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "commands/core/CommandFile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace keysplitter {


REGISTER_COMMANDPROVIDER(KeySplitterCommandProvider)

IMPLEMENT_LOADFROM(KeySplitterCommandProvider, KeySplitterCommand)

KeySplitterCommandProvider::KeySplitterCommandProvider()
{
}

KeySplitterCommandProvider::~KeySplitterCommandProvider()
{
}

CString KeySplitterCommandProvider::GetName()
{
	return _T("KeySplitterCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString KeySplitterCommandProvider::GetDisplayName()
{
	return _T("振り分け");
}

// コマンドの種類の説明を示す文字列を取得
CString KeySplitterCommandProvider::GetDescription()
{
	CString str;
	str += _T("コマンド実行時のキー押下状態単位でコマンドを割り当てることができます\n");
	str += _T("Enter/Shift-Enter/Ctrl-Enterごとに異なるコマンドを実行させることができます");

	return str;
}

// コマンド新規作成ダイアログ
bool KeySplitterCommandProvider::NewDialog(CommandParameter* param)
{
	return KeySplitterCommand::NewDialog(param);
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t KeySplitterCommandProvider::KeySplitterCommandProvider::GetOrder() const
{
	return 5000;
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t KeySplitterCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(KeySplitterCommand::TypeDisplayName());
	return 1;
}

} // end of namespace keysplitter
} // end of namespace commands
} // end of namespace launcherapp

