#include "pch.h"
#include "SpecialFolderFiles.h"
#include "utility/ShortcutFile.h"
#include <mutex>
#include <thread>
#include <deque>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace soyokaze {
namespace commands {
namespace specialfolderfiles {

static const int INTERVAL = 5000;

struct SpecialFolderFiles::PImpl
{
	std::mutex mMutex;
	std::vector<ITEM> mItems;
	DWORD mElapsed = 0;
	bool mIsUpdated = false;
};


SpecialFolderFiles::SpecialFolderFiles() : in(std::make_unique<PImpl>())
{
}

SpecialFolderFiles::~SpecialFolderFiles()
{
}

bool SpecialFolderFiles::GetShortcutFiles(std::vector<ITEM>& items)
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

		std::vector<ITEM> tmp;
		// 最近使ったファイルを得る
		GetFiles(tmp, CSIDL_RECENT);
		// ユーザのスタートメニュー
		GetFiles(tmp, CSIDL_STARTMENU);
		// すべてのユーザのスタートメニュー
		GetFiles(tmp, CSIDL_COMMON_STARTMENU);

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

void SpecialFolderFiles::GetFiles(std::vector<ITEM>& items, int csidl)
{
	TCHAR path[MAX_PATH_NTFS];
	SHGetSpecialFolderPath(NULL, path, csidl, 0);
	PathAppend(path, _T("*.*"));

	// フォルダ内のファイル列挙
	std::deque<CString> stk;
	stk.push_back(path);

	std::vector<ITEM> tmp;
	while(stk.empty() == false) {

		CString curDir = stk.front();
		stk.pop_front();

		CFileFind f;
		BOOL isLoop = f.FindFile(curDir, 0);
		while (isLoop) {

			Sleep(0);

			isLoop = f.FindNextFile();

			if (f.IsDots()) {
			 continue;
			}
	 
			if (f.IsDirectory()) {
				auto subDir = f.GetFilePath();
				PathAppend(subDir.GetBuffer(MAX_PATH_NTFS), _T("*.*"));
				subDir.ReleaseBuffer();
				stk.push_back(subDir);
				continue;
			}

			CString fileName = f.GetFileName();
			if (_tcsicmp(PathFindExtension(fileName), _T(".lnk")) != 0) {
				continue;
			}

			ITEM item;
			item.mType = csidl == CSIDL_RECENT ? TYPE_RECENT : TYPE_STARTMENU;
			item.mName = PathFindFileName(fileName);
			PathRemoveExtension(item.mName.GetBuffer(item.mName.GetLength()));   // .lnkを抜く
			item.mName.ReleaseBuffer();
			item.mFullPath = ShortcutFile::ResolvePath(f.GetFilePath(), &item.mDescription);

			if (item.mFullPath.IsEmpty()) {
				continue;
			}
			if (PathFileExists(item.mFullPath) == FALSE) {
				// 存在しないパスを除外する
				continue;
			}

			tmp.push_back(item);
		}
	}
	items.insert(items.end(), tmp.begin(), tmp.end());
}

} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace soyokaze

