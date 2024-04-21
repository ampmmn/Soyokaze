#pragma once

#include "commands/common/AdhocCommandProviderBase.h"
#include "commands/core/CommandProviderIF.h"

namespace launcherapp {
namespace commands {
namespace getip {


class GetIPCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	GetIPCommandProvider();
	virtual ~GetIPCommandProvider();

public:
	virtual CString GetName();

	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);


	DECLARE_COMMANDPROVIDER(GetIPCommandProvider)
};


} // end of namespace getip
} // end of namespace commands
} // end of namespace launcherapp

