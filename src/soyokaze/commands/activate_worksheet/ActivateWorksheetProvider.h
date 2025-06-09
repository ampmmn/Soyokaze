#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace activate_worksheet {


class ActivateWorksheetProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	ActivateWorksheetProvider();
	~ActivateWorksheetProvider() override;

public:
	CString GetName() override;

	// 一時的なコマンドの準備を行うための初期化
	void PrepareAdhocCommands() override;
	// 一時的なコマンドを必要に応じて提供する
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;

	DECLARE_COMMANDPROVIDER(ActivateWorksheetProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace activate_worksheet
} // end of namespace commands
} // end of namespace launcherapp

