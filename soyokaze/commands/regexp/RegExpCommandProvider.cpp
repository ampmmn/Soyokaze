#include "pch.h"
#include "RegExpCommandProvider.h"
#include "commands/regexp/RegExpCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
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
	return CString((LPCTSTR)IDS_DESCRIPTION_REGEXPCOMMAND);
}

// コマンド新規作成ダイアログ
bool RegExpCommandProvider::NewDialog(const CommandParameter* param)
{
	return RegExpCommand::NewDialog(param);
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t RegExpCommandProvider::RegExpCommandProvider::GetOrder() const
{
	return 150;
}

}
}
}
