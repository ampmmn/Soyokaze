#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ContextMenuSourceIF.h"
#include "commands/mmc/MMCSnapin.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace mmc {

class MMCCommand :
 	public launcherapp::commands::common::AdhocCommandBase,
	public launcherapp::commands::core::ContextMenuSource
{
public:
	MMCCommand(const MMCSnapin& snapin);
	virtual ~MMCCommand();

	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ContextMenuSource
	// メニューの項目数を取得する
	int GetMenuItemCount() override;
	// メニューに対応するアクションを取得する
	bool GetMenuItem(int index, Action** action) override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	static CString TypeDisplayName();

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(MMCCommand)
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace mmc
} // end of namespace commands
} // end of namespace launcherapp

