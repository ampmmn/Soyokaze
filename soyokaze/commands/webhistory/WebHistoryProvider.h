#pragma once

#include "commands/common/UserCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace webhistory {

class WebHistoryProvider :
	public launcherapp::commands::common::UserCommandProviderBase
{
private:
	WebHistoryProvider();
	~WebHistoryProvider() override;

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

	DECLARE_COMMANDPROVIDER(WebHistoryProvider)

// UserCommandProviderBase
	bool LoadFrom(CommandEntryIF* entry, Command** command) override;
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp

