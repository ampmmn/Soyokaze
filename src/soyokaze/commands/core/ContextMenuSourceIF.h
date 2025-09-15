#pragma once

#include "core/UnknownIF.h"

#include "commands/core/CommandParameter.h"

namespace launcherapp {
namespace commands {
namespace core {

class ContextMenuSource : virtual public launcherapp::core::UnknownIF
{
public:
	// メニューの項目数を取得する
	virtual int GetMenuItemCount() = 0;
	// メニューの表示名を取得する
	virtual bool GetMenuItemName(int index, LPCWSTR* displayNamePtr) = 0;
	// メニュー選択時の処理を実行する
	virtual bool SelectMenuItem(int index, launcherapp::core::CommandParameter* param) = 0;
};

} // end of namespace core
} // end of namespace commands
} // end of namespace launcherapp
