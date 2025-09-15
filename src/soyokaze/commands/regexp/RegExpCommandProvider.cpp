#include "pch.h"
#include "RegExpCommandProvider.h"
#include "commands/regexp/RegExpCommand.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace regexp {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(RegExpCommandProvider)

IMPLEMENT_LOADFROM(RegExpCommandProvider, RegExpCommand)

RegExpCommandProvider::RegExpCommandProvider()
{
}

RegExpCommandProvider::~RegExpCommandProvider()
{
}

CString RegExpCommandProvider::GetName()
{
	return _T("RegExpCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString RegExpCommandProvider::GetDisplayName()
{
	return CString((LPCTSTR)IDS_REGEXPCOMMAND);
}

// コマンドの種類の説明を示す文字列を取得
CString RegExpCommandProvider::GetDescription()
{
	static LPCTSTR description = _T("正規表現によるコマンド名のマッチングを行うことができます。\n")
		_T("一致したテキストからグループを取り出して、後段の処理に渡すことができます\n")
		_T("活用例として、課題管理システムの識別子から該当するURLを開くような使い方ができます\n")
		_T("(メールやチャット上のチケット識別子をコピペしてチケット画面を開く、のようなことができる)");
	return description;
}

// コマンド新規作成ダイアログ
bool RegExpCommandProvider::NewDialog(Parameter* param)
{
	return RegExpCommand::NewDialog(param);
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t RegExpCommandProvider::RegExpCommandProvider::GetOrder() const
{
	return 150;
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t RegExpCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(RegExpCommand::TypeDisplayName());
	return 1;
}

}
}
}
