#pragma once

#include "commands/core/CommandQueryItemList.h"
#include <cstddef>
#include <memory>

namespace launcherapp { namespace commands { namespace bookmarks {

// 元はユーザ登録型のコマンドとして実装していたが、ただの内部クラス
class BookmarkCommand
{
public:
	struct Statistics
	{
		std::size_t edgeItemCapacity{0};
		std::size_t alternativeBookmarkItemCapacity{0};
	};

	BookmarkCommand();
	~BookmarkCommand();

	bool Load();
	bool QueryCandidates(Pattern* pattern, launcherapp::CommandQueryItemList& commands);
	static Statistics GetStatistics();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::bookmarks

