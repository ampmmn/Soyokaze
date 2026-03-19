#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace launcherapp { namespace commands { namespace win32menu {


class Win32MenuCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	Win32MenuCommandProvider();
	~Win32MenuCommandProvider() override;

public:
	CString GetName() override;

	// 一時的なコマンドを必要に応じて提供する
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;

	DECLARE_COMMANDPROVIDER(Win32MenuCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::win32menu

