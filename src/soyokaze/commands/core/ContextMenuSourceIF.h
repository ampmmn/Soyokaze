#pragma once

#include "core/UnknownIF.h"

#include "actions/core/ActionParameter.h"

namespace launcherapp {
namespace commands {
namespace core {

class ContextMenuSource : virtual public launcherapp::core::UnknownIF
{
public:
	// メニューの項目数を取得する
	virtual int GetMenuItemCount() = 0;
	// メニューに対応するアクションを取得する
	virtual bool GetMenuItem(int index, actions::core::Action** action) = 0;
};

} // end of namespace core
} // end of namespace commands
} // end of namespace launcherapp
