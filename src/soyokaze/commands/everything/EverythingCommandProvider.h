#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace launcherapp { namespace commands { namespace everything {


class EverythingCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	EverythingCommandProvider();
	~EverythingCommandProvider() override;

public:
	CString GetName() override;

	// 一時的なコマンドの準備を行うための初期化
	void PrepareAdhocCommands() override;
	// Provider間の優先順位を表す値を返す。小さいほど優先
	uint32_t GetOrder() const override;
	// 一時的なコマンドを必要に応じて提供する
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;
	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(EverythingCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::everything

