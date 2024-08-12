#pragma once

#include "commands/common/RoutineCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace bookmarks {

class BookmarkCommandProvider :
	public launcherapp::commands::common::RoutineCommandProviderBase
{
private:
	BookmarkCommandProvider();
	~BookmarkCommandProvider() override;

public:
	CString GetName() override;

	// 作成できるコマンドの種類を表す文字列を取得
	CString GetDisplayName() override;

	// コマンドの種類の説明を示す文字列を取得
	CString GetDescription() override;

	// コマンド新規作成ダイアログ
	bool NewDialog(const CommandParameter* param) override;

	// 一時的なコマンドを必要に応じて提供する
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;

	// Provider間の優先順位を表す値を返す。小さいほど優先
	uint32_t GetOrder() const override;

	DECLARE_COMMANDPROVIDER(BookmarkCommandProvider)

// RoutineCommandProviderBase
	bool LoadFrom(CommandEntryIF* entry, Command** command) override;
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

