#include "pch.h"
#include "ActivateWindowProvider.h"
#include "commands/activate_window/WindowActivateAdhocCommand.h"
#include "commands/activate_window/WindowActivateCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandFile.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace activate_window {

using CommandRepository = launcherapp::core::CommandRepository;

struct ActivateWindowProvider::PImpl
{
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(ActivateWindowProvider)

IMPLEMENT_LOADFROM(ActivateWindowProvider, WindowActivateCommand)

ActivateWindowProvider::ActivateWindowProvider() : in(std::make_unique<PImpl>())
{
}

ActivateWindowProvider::~ActivateWindowProvider()
{
}

CString ActivateWindowProvider::GetName()
{
	return _T("ActiveWindowCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString ActivateWindowProvider::GetDisplayName()
{
	return CString((LPCTSTR)IDS_COMMANDNAME_WINDOWACTIVATE);
}

// コマンドの種類の説明を示す文字列を取得
CString ActivateWindowProvider::GetDescription()
{
	CString description((LPCTSTR)IDS_DESCRIPTION_WINDOWACTIVATE);
	description += _T("\n");
	description += _T("ウインドウ切り替え処理に対して、任意のキーワードを設定したり、\n");
	description += _T("ホットキーを設定することができます。\n");
	return description;
}

// コマンド新規作成ダイアログ
bool ActivateWindowProvider::NewDialog(Parameter* param)
{
	WindowActivateCommand* newCmd{nullptr};
	if (WindowActivateCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}

	CommandRepository::GetInstance()->RegisterCommand(newCmd);
	return true;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t ActivateWindowProvider::GetOrder() const
{
	return 500;
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t ActivateWindowProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(WindowActivateCommand::TypeDisplayName());
	displayNames.push_back(WindowActivateAdhocCommand::TypeDisplayName());
	return 2;
}

} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

