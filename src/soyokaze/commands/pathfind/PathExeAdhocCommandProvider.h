#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace pathfind {


class PathExeAdhocCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	PathExeAdhocCommandProvider();
	virtual ~PathExeAdhocCommandProvider();

public:
	void AddHistory(const CString& word, const CString& fullPath);

public:
	// コマンドの読み込み
	virtual void LoadCommands(CommandFile* commandFile);

	virtual CString GetName();
	// 一時的なコマンドの準備を行うための初期化
	void PrepareAdhocCommands() override;
	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);
	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;
	// Provider間の優先順位を表す値を返す。小さいほど優先
	uint32_t GetOrder() const override;

	DECLARE_COMMANDPROVIDER(PathExeAdhocCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace launcherapp

