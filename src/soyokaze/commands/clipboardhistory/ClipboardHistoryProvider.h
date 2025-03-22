#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

class ClipboardHistoryProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	ClipboardHistoryProvider();
	~ClipboardHistoryProvider() override;

public:
	CString GetName() override;

	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);

	// 設定ページを取得する
	bool CreateSettingPages(CWnd* parent, std::vector<SettingPage*>& pages) override;

	DECLARE_COMMANDPROVIDER(ClipboardHistoryProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp

