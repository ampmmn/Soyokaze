#pragma once

#include "commands/common/UserCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace bookmarks {

class BookmarkCommandProvider :
	public launcherapp::commands::common::UserCommandProviderBase
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
	bool NewDialog(CommandParameter* param) override;

	// Provider間の優先順位を表す値を返す。小さいほど優先
	uint32_t GetOrder() const override;

	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(BookmarkCommandProvider)

// UserCommandProviderBase
	void OnBeforeLoad() override;
	bool LoadFrom(CommandEntryIF* entry, Command** command) override;
};


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

