#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace controlpanel {

class ControlPanelProvider : 
	public launcherapp::commands::common::AdhocCommandProviderBase
{
public:
	ControlPanelProvider();
	virtual ~ControlPanelProvider();

public:
	CString GetName() override;

	// 一時的なコマンドの準備を行うための初期化
	void PrepareAdhocCommands() override;
	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);

	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(ControlPanelProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}
}


