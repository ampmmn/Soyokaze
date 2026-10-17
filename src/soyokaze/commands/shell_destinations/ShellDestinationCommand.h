#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ContextMenuSourceIF.h"
#include "commands/shell_destinations/AutomaticDestinationsFile.h"
#include <memory>

namespace launcherapp { namespace commands { namespace shell_destinations {

/**
  ジャンプリストのピン留め項目1件に対応するコマンド
*/
class ShellDestinationCommand :
	public launcherapp::commands::common::AdhocCommandBase,
	public launcherapp::commands::core::ContextMenuSource
{
public:
	ShellDestinationCommand(const AutomaticDestinationEntry& entry);
	virtual ~ShellDestinationCommand();

	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	int GetMenuItemCount() override;
	bool GetMenuItem(int index, Action** action) override;
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(ShellDestinationCommand)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}}}
