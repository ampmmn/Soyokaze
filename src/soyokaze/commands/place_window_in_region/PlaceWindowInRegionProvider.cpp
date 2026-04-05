#include "pch.h"
#include "PlaceWindowInRegionProvider.h"
#include "commands/place_window_in_region/PlaceWindowInRegionCommand.h"
#include "commands/place_window_in_region/PlaceWindowInRegionAdhocCommand.h"
#include "commands/place_window_in_region/PlaceWindowInRegionSettingDialog.h"
#include "commands/activate_window/WindowList.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "mainwindow/LauncherWindowEventDispatcher.h"
#include "mainwindow/controller/MainWindowController.h"
#include "resource.h"
#include <map>
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace place_window_in_region {

using CommandRepository = launcherapp::core::CommandRepository;

struct PlaceWindowInRegionProvider::PImpl
{
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(PlaceWindowInRegionProvider)

IMPLEMENT_LOADFROM(PlaceWindowInRegionProvider, PlaceWindowInRegionCommand)

PlaceWindowInRegionProvider::PlaceWindowInRegionProvider() : in(std::make_unique<PImpl>())
{
}

PlaceWindowInRegionProvider::~PlaceWindowInRegionProvider()
{
}

CString PlaceWindowInRegionProvider::GetName()
{
	return _T("PlaceWindowInRegionCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString PlaceWindowInRegionProvider::GetDisplayName()
{
	return CString(_T("ウインドウ配置コマンド"));
}

// コマンドの種類の説明を示す文字列を取得
CString PlaceWindowInRegionProvider::GetDescription()
{
	return _T("あらかじめ設定した領域に対し、任意のウインドウを配置するコマンドです。");
}

// コマンド新規作成ダイアログ
bool PlaceWindowInRegionProvider::NewDialog(Parameter* param)
{
	PlaceWindowInRegionCommand* newCmd{nullptr};
	if (PlaceWindowInRegionCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}

	CommandRepository::GetInstance()->RegisterCommand(newCmd);
	return true;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t PlaceWindowInRegionProvider::GetOrder() const
{
	return 1600;
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t PlaceWindowInRegionProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(PlaceWindowInRegionCommand::TypeDisplayName());
	displayNames.push_back(PlaceWindowInRegionAdhocCommand::TypeDisplayName());
	return 2;
}

} // end of namespace place_window_in_region
} // end of namespace commands
} // end of namespace launcherapp

