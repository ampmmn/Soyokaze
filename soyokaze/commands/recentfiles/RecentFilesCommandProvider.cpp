#include "pch.h"
#include "RecentFilesCommandProvider.h"
#include "commands/recentfiles/RecentFileCommand.h"
#include "commands/recentfiles/RecentFiles.h"
#include "core/CommandRepository.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace recentfiles {

using CommandRepository = soyokaze::core::CommandRepository;

struct RecentFilesCommandProvider::PImpl
{
	PImpl()
	{
	}
	virtual ~PImpl()
	{
	}

	std::vector<ITEM> mRecentFileItems;
	RecentFiles mFiles;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(RecentFilesCommandProvider)


RecentFilesCommandProvider::RecentFilesCommandProvider() : in(new PImpl)
{
}

RecentFilesCommandProvider::~RecentFilesCommandProvider()
{
}

CString RecentFilesCommandProvider::GetName()
{
	return _T("RecentFiles");
}

// 一時的なコマンドを必要に応じて提供する
void RecentFilesCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	std::vector<CommandQueryItem>& commands
)
{
	in->mFiles.GetRecentFiles(in->mRecentFileItems);

	for (auto item : in->mRecentFileItems) {

		const auto& name = item.mName;

		int level = pattern->Match(name);
		if (level == Pattern::Mismatch) {
			continue;
		}
		commands.push_back(CommandQueryItem(level, new RecentFileCommand(name, item.mFullPath)));
	}
}


} // end of namespace recentfiles
} // end of namespace commands
} // end of namespace soyokaze

