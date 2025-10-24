#pragma once

#include "commands/common/AdhocCommandProviderBase.h"
#include <memory>

namespace launcherapp { namespace commands { namespace outlook {

class OutlookFolderProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	OutlookFolderProvider();
	virtual ~OutlookFolderProvider();

public:
	virtual CString GetName();

	// 一時的なコマンドの準備を行うための初期化
	void PrepareAdhocCommands() override;
	// 一時的なコマンドを必要に応じて提供する
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;
	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(OutlookFolderProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



}}}

