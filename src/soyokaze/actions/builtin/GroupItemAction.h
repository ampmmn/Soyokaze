#pragma once

#include "actions/core/ActionBase.h"
#include "hotkey/HotKeyAttribute.h"
#include "commands/group/CommandParam.h"

class HOTKEY_ATTR;

namespace launcherapp { namespace actions { namespace builtin {

// グループ内の一要素を種別に応じて実行するアクション
class GroupItemAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	GroupItemAction(const CString& parentName, const launcherapp::commands::group::GroupItem& item,
		const HOTKEY_ATTR& hotkeyAttr);
	~GroupItemAction();

	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}}}
