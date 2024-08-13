#pragma once

#include "commands/common/RoutineCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace url_directoryindex {


class URLDirectoryIndexCommandProvider :
	public launcherapp::commands::common::RoutineCommandProviderBase
{

private:
	URLDirectoryIndexCommandProvider();
	~URLDirectoryIndexCommandProvider() override;

public:
	virtual CString GetName();

	// 作成できるコマンドの種類を表す文字列を取得
	virtual CString GetDisplayName();

	// コマンドの種類の説明を示す文字列を取得
	virtual CString GetDescription();

	// コマンド新規作成ダイアログ
	virtual bool NewDialog(const CommandParameter* param);

	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);

	// Provider間の優先順位を表す値を返す。小さいほど優先
	virtual uint32_t GetOrder() const;

	DECLARE_COMMANDPROVIDER(URLDirectoryIndexCommandProvider)

// RoutineCommandProviderBase
	bool LoadFrom(CommandEntryIF* entry, Command** command) override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

