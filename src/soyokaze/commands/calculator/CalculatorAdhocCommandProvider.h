#pragma once

#include "commands/common/AdhocCommandProviderBase.h"
#include "commands/core/CommandProviderIF.h"


namespace launcherapp {
namespace commands {
namespace calculator {


class CalculatorAdhocCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	CalculatorAdhocCommandProvider();
	virtual ~CalculatorAdhocCommandProvider();

public:
	virtual CString GetName();

	// 一時的なコマンドの準備を行うための初期化
	void PrepareAdhocCommands() override;
	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);


	DECLARE_COMMANDPROVIDER(CalculatorAdhocCommandProvider)

	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace calculator
} // end of namespace commands
} // end of namespace launcherapp

