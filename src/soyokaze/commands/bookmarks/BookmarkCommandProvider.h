#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace bookmarks {

class BookmarkCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	BookmarkCommandProvider();
	~BookmarkCommandProvider() override;

public:
	CString GetName() override;

	// 一時的なコマンドの準備を行うための初期化
	void PrepareAdhocCommands() override;
	// 一時的なコマンドを必要に応じて提供する
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;
	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(BookmarkCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

