#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace launcherapp { namespace commands { namespace uiautomation {


class UIAutomationCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	UIAutomationCommandProvider();
	~UIAutomationCommandProvider() override;

public:
	CString GetName() override;

	// 一時的なコマンドを必要に応じて提供する
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;

	DECLARE_COMMANDPROVIDER(UIAutomationCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::uiautomation

