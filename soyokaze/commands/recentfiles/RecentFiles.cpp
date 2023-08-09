#include "pch.h"
#include "RecentFiles.h"
#include "utility/ShortcutFile.h"
#include <mutex>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace soyokaze {
namespace commands {
namespace recentfiles {

static const int INTERVAL = 5000;

struct RecentFiles::PImpl
{
	std::mutex mMutex;
	std::vector<ITEM> mItems;
	DWORD mElapsed = 0;
	bool mIsUpdated = false;
};


RecentFiles::RecentFiles() : in(new PImpl)
{
}

RecentFiles::~RecentFiles()
{
}

bool RecentFiles::GetRecentFiles(std::vector<ITEM>& items)
{
	{
		std::lock_guard<std::mutex> lock(in->mMutex);
		DWORD elapsed = GetTickCount() - in->mElapsed;
		if (elapsed <= INTERVAL) {
			if (in->mIsUpdated) {
				items = in->mItems;
				in->mIsUpdated = false;
				return true;
			}
			return false;
		}
	}

	std::thread th([&]() {
		
		CoInitialize(NULL);

		TCHAR path[MAX_PATH_NTFS];
		SHGetSpecialFolderPath(NULL, path, CSIDL_RECENT, 0);
		PathAppend(path, _T("*.lnk"));

		// フォルダ内のファイル列挙
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

		CoUninitialize();

		std::lock_guard<std::mutex> lock(in->mMutex);
		in->mItems.swap(tmp);
		in->mElapsed = GetTickCount();
		in->mIsUpdated = true;
	});
	th.detach();

	std::lock_guard<std::mutex> lock(in->mMutex);
	if (in->mIsUpdated) {
		items = in->mItems;
		in->mIsUpdated = false;
		return true;
	}
	return false;
}


} // end of namespace recentfiles
} // end of namespace commands
} // end of namespace soyokaze

