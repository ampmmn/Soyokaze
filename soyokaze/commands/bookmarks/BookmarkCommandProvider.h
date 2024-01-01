#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace soyokaze {
namespace commands {
namespace bookmarks {

class BookmarkCommandProvider :
	public soyokaze::commands::common::AdhocCommandProviderBase
{
private:
	BookmarkCommandProvider();
	virtual ~BookmarkCommandProvider();

public:
	virtual CString GetName();

	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);

	// 設定ページを取得する
	bool CreateSettingPages(CWnd* parent, std::vector<SettingPage*>& pages) override;

	DECLARE_COMMANDPROVIDER(BookmarkCommandProvider)

private:
	void QueryBookmarks(Pattern* pattern, CommandQueryItemList& comands);
	void QueryHistories(Pattern* pattern, CommandQueryItemList& comands);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace soyokaze

