#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace mssettings {

class MSSettingsCommandProvider : 
	public launcherapp::commands::common::AdhocCommandProviderBase
{
public:
	MSSettingsCommandProvider();
	virtual ~MSSettingsCommandProvider();

public:
	CString GetName() override;

	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);

	DECLARE_COMMANDPROVIDER(MSSettingsCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}
}


