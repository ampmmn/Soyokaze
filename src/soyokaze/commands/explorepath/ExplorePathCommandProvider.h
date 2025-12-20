#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace launcherapp { namespace commands { namespace explorepath {

class ExplorePathCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	ExplorePathCommandProvider();
	virtual ~ExplorePathCommandProvider();

public:
	void AddHistory(const CString& word, const CString& fullPath);

public:
	virtual CString GetName();
	// 一時的なコマンドの準備を行うための初期化
	void PrepareAdhocCommands() override;
	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);
	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;
	// Provider間の優先順位を表す値を返す。小さいほど優先
	uint32_t GetOrder() const override;

	DECLARE_COMMANDPROVIDER(ExplorePathCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::pathfind

