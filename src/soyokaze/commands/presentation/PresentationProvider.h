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

	// 一時的なコマンドの準備を行うための初期化
	void PrepareAdhocCommands() override;
	// 一時的なコマンドを必要に応じて提供する
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;
	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(PresentationProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



} // end of namespace presentation
} // end of namespace commands
} // end of namespace launcherapp

