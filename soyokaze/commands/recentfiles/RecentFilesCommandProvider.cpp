#include "pch.h"
#include "RecentFilesCommandProvider.h"
#include "commands/recentfiles/RecentFileCommand.h"
#include "core/CommandRepository.h"
#include "utility/ShortcutFile.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace recentfiles {

static const int INTERVAL = 5000;

using CommandRepository = soyokaze::core::CommandRepository;

struct ITEM {
	CString mName;
	CString mFullPath;
};

struct RecentFilesCommandProvider::PImpl
{
	PImpl()
	{
	}
	virtual ~PImpl()
	{
	}

	DWORD mLastUpdated = 0;

	std::vector<ITEM> mRecentFileItems;
	DWORD mElapsed;

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
	CString cmdline = pattern->GetWholeString();

	DWORD elapsed = GetTickCount() - in->mElapsed;
	if (elapsed > INTERVAL) {

		TCHAR path[MAX_PATH_NTFS];
		SHGetSpecialFolderPath(NULL, path, CSIDL_RECENT, 0);
		PathAppend(path, _T("*.lnk"));

		// ToDo: フォルダ内のファイル列挙
	std::vector<ITEM> tmp;
		CFileFind f;
		BOOL isLoop = f.FindFile(path, 0);
		while (isLoop) {
			isLoop = f.FindNextFile();
			if (f.IsDots() || f.IsDirectory()) {
				continue;
			}

			ITEM item;
			item.mName = PathFindFileName(f.GetFileName());
			PathRemoveExtension(item.mName.GetBuffer(item.mName.GetLength()));   // .lnkを抜く
			item.mName.ReleaseBuffer();
			item.mFullPath = ShortcutFile::ResolvePath(f.GetFilePath());

			if (item.mFullPath.IsEmpty()) {
				continue;
			}

			tmp.push_back(item);
		}
		in->mRecentFileItems.swap(tmp);
		in->mElapsed = GetTickCount();
	}

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

