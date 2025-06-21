#include "pch.h"
#include "AlignWindowProvider.h"
#include "commands/align_window/AlignWindowCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace align_window {

using CommandRepository = launcherapp::core::CommandRepository;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(AlignWindowProvider)

IMPLEMENT_LOADFROM(AlignWindowProvider, AlignWindowCommand)

AlignWindowProvider::AlignWindowProvider()
{
}

AlignWindowProvider::~AlignWindowProvider()
{
}

CString AlignWindowProvider::GetName()
{
	return _T("AlignWindowCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString AlignWindowProvider::GetDisplayName()
{
	return CString(_T("ウインドウ整列コマンド"));
}

// コマンドの種類の説明を示す文字列を取得
CString AlignWindowProvider::GetDescription()
{
	CString description;
	description += _T("1つ以上のウインドウを整列するコマンドを作成します。\n");
	description += _T("条件に合致するウインドウの位置やサイズを整えることができます。\n");
	return description;
}

// コマンド新規作成ダイアログ
bool AlignWindowProvider::NewDialog(CommandParameter* param)
{
	AlignWindowCommand* newCmd = nullptr;
	if (AlignWindowCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}

	CommandRepository::GetInstance()->RegisterCommand(newCmd);
	return true;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t AlignWindowProvider::GetOrder() const
{
	return 1500;
}

} // end of namespace align_window
} // end of namespace commands
} // end of namespace launcherapp

