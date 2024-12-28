#pragma once

#include "commands/common/AdhocCommandProviderBase.h"


namespace launcherapp {
namespace commands {
namespace presentation {

class PresentationProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	PresentationProvider();
	virtual ~PresentationProvider();

public:
	virtual CString GetName();

	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);

	DECLARE_COMMANDPROVIDER(PresentationProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



} // end of namespace presentation
} // end of namespace commands
} // end of namespace launcherapp

