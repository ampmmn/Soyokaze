#pragma once

#include "commands/bookmarks/Bookmarks.h"
#include "commands/core/CommandQueryItemList.h"
#include <memory>

namespace launcherapp { namespace commands { namespace bookmarks {

// 元はユーザ登録型のコマンドとして実装していたが、ただの内部クラス
class BookmarkCommand
{
public:
	BookmarkCommand();
	~BookmarkCommand();

	bool Load();
	bool QueryCandidates(Pattern* pattern, launcherapp::CommandQueryItemList& commands);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::bookmarks

