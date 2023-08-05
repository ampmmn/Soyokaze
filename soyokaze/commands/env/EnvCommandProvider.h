#pragma once

#include "commands/common/AdhocCommandProviderBase.h"
#include "core/CommandProviderIF.h"


namespace soyokaze {
namespace commands {
namespace env {


class EnvCommandProvider :
	public soyokaze::commands::common::AdhocCommandProviderBase
{
private:
	EnvCommandProvider();
	virtual ~EnvCommandProvider();

public:
	virtual CString GetName();

	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, std::vector<CommandQueryItem>& comands);


	DECLARE_COMMANDPROVIDER(EnvCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace env
} // end of namespace commands
} // end of namespace soyokaze
