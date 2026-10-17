#pragma once

#include "commands/common/AdhocCommandProviderBase.h"
#include <memory>
#include <vector>

namespace launcherapp { namespace commands { namespace shell_destinations {

/**
  AutomaticDestinations のピン留め項目から一時コマンドを生成する
*/
class ShellDestinationCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
public:
	ShellDestinationCommandProvider();
	virtual ~ShellDestinationCommandProvider();

	CString GetName() override;
	void PrepareAdhocCommands() override;
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands) override;
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(ShellDestinationCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}}}
