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
	virtual void QueryAdhocCommands(Pattern* pattern, std::vector<CommandQueryItem>& comands);

	DECLARE_COMMANDPROVIDER(BookmarkCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace soyokaze

