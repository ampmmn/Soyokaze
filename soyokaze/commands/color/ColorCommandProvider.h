#pragma once

#include "commands/common/AdhocCommandProviderBase.h"
#include "commands/core/CommandProviderIF.h"


namespace soyokaze {
namespace commands {
namespace color {


class ColorCommandProvider :
	public soyokaze::commands::common::AdhocCommandProviderBase
{
private:
	ColorCommandProvider();
	virtual ~ColorCommandProvider();

public:
	virtual CString GetName();

	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);


	DECLARE_COMMANDPROVIDER(ColorCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace color
} // end of namespace commands
} // end of namespace soyokaze

