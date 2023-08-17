#pragma once

#include "commands/common/AdhocCommandProviderBase.h"
#include "core/CommandProviderIF.h"


namespace soyokaze {
namespace commands {
namespace calculator {


class CalculatorAdhocCommandProvider :
	public soyokaze::commands::common::AdhocCommandProviderBase
{
private:
	CalculatorAdhocCommandProvider();
	virtual ~CalculatorAdhocCommandProvider();

public:
	virtual CString GetName();

	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);


	DECLARE_COMMANDPROVIDER(CalculatorAdhocCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace calculator
} // end of namespace commands
} // end of namespace soyokaze

