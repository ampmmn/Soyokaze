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
	CString GetName() override;

	// 一時的なコマンドを必要に応じて提供する
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;
	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;


	DECLARE_COMMANDPROVIDER(GetIPCommandProvider)
};


} // end of namespace getip
} // end of namespace commands
} // end of namespace launcherapp

