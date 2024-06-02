#pragma once

#include "commands/common/AdhocCommandProviderBase.h"
#include "commands/core/CommandProviderIF.h"


namespace launcherapp {
namespace commands {
namespace mmc {


class MMCCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	MMCCommandProvider();
	virtual ~MMCCommandProvider();

public:
	CString GetName() override;

	// 一時的なコマンドを必要に応じて提供する
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;


	DECLARE_COMMANDPROVIDER(MMCCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace mmc
} // end of namespace commands
} // end of namespace launcherapp

