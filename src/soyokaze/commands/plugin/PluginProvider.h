#pragma once

#include "commands/common/AdhocCommandProviderBase.h"
#include "commands/core/CommandProviderIF.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace plugin {

class PluginProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	PluginProvider();
	~PluginProvider() override;

public:
	CString GetName() override;
	void PrepareAdhocCommands() override;
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands) override;
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(PluginProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // namespace plugin
} // namespace commands
} // namespace launcherapp
