#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/plugin/PluginModule.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace plugin {

class PluginCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	PluginCommand(const PluginModulePtr& module, const PluginMatchPtr& match, int index,
	              const CString& name, const CString& description);
	~PluginCommand() override;

	CString GetTypeDisplayName() override;
	bool CanExecute(String* reasonMsg) override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(PluginCommand)

private:
	PluginModulePtr mModule;
	PluginMatchPtr mMatch;
	int mIndex{0};
	CString mTypeDisplayName;
	HICON mIcon{nullptr};
};

} // namespace plugin
} // namespace commands
} // namespace launcherapp
