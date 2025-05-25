#include "pch.h"
#include "SpecialFolderFiles.h"
#include "utility/ShortcutFile.h"
#include "utility/Path.h"
#include "utility/LocalDirectoryWatcher.h"
#include <mutex>
#include <deque>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace specialfolderfiles {

struct SpecialFolderFiles::PImpl
{
	void RegisterWatcher();
	void GetLnkFiles(std::vector<ITEM>& items, const CString& directoryPath, int type);

	std::mutex mMutex;

	std::vector<ITEM> mRecentItems;
	std::vector<ITEM> mStartMenuItems;
	std::vector<ITEM> mCommonStartMenuItems;

	// 最近使ったファイル
	CString mRecentPath;
	// スタートメニュー
	CString mStartMenuPath;
	// すべてのユーザーのスタートメニュー
	CString mCommonStartMenuPath;

	bool mIsFirstCall{true};
	bool mIsEnableRecent{true};
	bool mIsEnableStartMenu{true};
};

void SpecialFolderFiles::PImpl::RegisterWatcher()
{
	auto watcher = LocalDirectoryWatcher::GetInstance();

	Path path;

	// 最近使ったファイルのフォルダが更新されたら通知を受け取るための登録をする
	SHGetSpecialFolderPath(NULL, (LPTSTR)path, CSIDL_RECENT, 0);
	mRecentPath = path;
	watcher->Register(mRecentPath, [](void* p) {
			auto thisPtr = (PImpl*)p;
			if (thisPtr->mIsEnableRecent == false) {
				return;
			}
			std::vector<ITEM> items;
			thisPtr->GetLnkFiles(items, thisPtr->mRecentPath, TYPE_RECENT);

			std::lock_guard<std::mutex> lock(thisPtr->mMutex);
			thisPtr->mRecentItems.swap(items);
	}, this);

	// スタートメニューフォルダが更新されたら通知を受け取るための登録をする
	SHGetSpecialFolderPath(NULL, (LPTSTR)path, CSIDL_STARTMENU, 0);
	mStartMenuPath = path;
	watcher->Register(mStartMenuPath, [](void* p) {
			auto thisPtr = (PImpl*)p;
			if (thisPtr->mIsEnableStartMenu == false) {
				return;
			}
			std::vector<ITEM> items;
			thisPtr->GetLnkFiles(items, thisPtr->mStartMenuPath, TYPE_STARTMENU);

			std::lock_guard<std::mutex> lock(thisPtr->mMutex);
			thisPtr->mStartMenuItems.swap(items);
	}, this);


	// すべてのユーザーのスタートメニューフォルダが更新されたら通知を受け取るための登録をする
	SHGetSpecialFolderPath(NULL, (LPTSTR)path, CSIDL_COMMON_STARTMENU, 0);
	mCommonStartMenuPath = path;
	watcher->Register(mCommonStartMenuPath, [](void* p) {
			auto thisPtr = (PImpl*)p;
			if (thisPtr->mIsEnableStartMenu == false) {
				return;
			}
			std::vector<ITEM> items;
			thisPtr->GetLnkFiles(items, thisPtr->mCommonStartMenuPath, TYPE_STARTMENU);

			std::lock_guard<std::mutex> lock(thisPtr->mMutex);
			thisPtr->mCommonStartMenuItems.swap(items);
	}, this);

}

static bool GetLastWriteTime(const CString& path, FILETIME& tm)
{
	HANDLE h = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}
	BOOL isOK = GetFileTime(h, nullptr, nullptr, &tm);
	CloseHandle(h);

	return isOK != FALSE;
}

void SpecialFolderFiles::PImpl::GetLnkFiles(std::vector<ITEM>& items, const CString& directoryPath, int type)
{
	Path path(directoryPath);
	path.Append(_T("*.*"));

	// フォルダ内のファイル列挙
	std::deque<CString> stk;
	stk.push_back((LPTSTR)path);

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
	 
			if (f.IsHidden()) {
				// 隠しファイル属性があるものは対象外にする
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
			item.mType = type;
			item.mName = PathFindFileName(fileName);

			PathRemoveExtension(item.mName.GetBuffer(item.mName.GetLength()));   // .lnkを抜く
			item.mName.ReleaseBuffer();
			item.mFullPath = ShortcutFile::ResolvePath(f.GetFilePath(), &item.mDescription);
			item.mLinkPath = f.GetFilePath();

			if (item.mFullPath.IsEmpty()) {
				continue;
			}
			if (Path::FileExists(item.mFullPath) == FALSE) {
				// 存在しないパスを除外する
				continue;
			}

			tmp.push_back(item);
		}
		f.Close();
	}

	for (auto& item : tmp) {
		GetLastWriteTime(item.mFullPath, item.mWriteTime);
	}

	// 更新日時降順でソート
	std::sort(tmp.begin(), tmp.end(), [](const ITEM& l_, const ITEM& r_) {
		auto& l = l_.mWriteTime;
		auto& r = r_.mWriteTime;
		if (r.dwHighDateTime < l.dwHighDateTime) { return true; }
		if (r.dwHighDateTime > l.dwHighDateTime) { return false; }
		return r.dwLowDateTime < l.dwLowDateTime;
	});

	items.insert(items.end(), tmp.begin(), tmp.end());
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



SpecialFolderFiles::SpecialFolderFiles() : in(std::make_unique<PImpl>())
{
}

SpecialFolderFiles::~SpecialFolderFiles()
{
}

void SpecialFolderFiles::EnableStartMenu(bool isEnable)
{
	in->mIsEnableStartMenu = isEnable;
}

void SpecialFolderFiles::EnableRecent(bool isEnable)
{
	in->mIsEnableRecent = isEnable;
}

bool SpecialFolderFiles::GetShortcutFiles(std::vector<ITEM>& items)
{
	if (in->mIsFirstCall) {

		// 変更通知を受け取るための登録
		in->RegisterWatcher();

		// 初回はショートカット一覧を直接取得する。以降は変更通知経由で更新する
		std::thread th([&](){
			if (in->mIsEnableRecent) {
				std::lock_guard<std::mutex> lock(in->mMutex);
				in->GetLnkFiles(in->mRecentItems, in->mRecentPath, TYPE_RECENT);
			}
			if (in->mIsEnableStartMenu) {
				std::lock_guard<std::mutex> lock(in->mMutex);
				in->GetLnkFiles(in->mStartMenuItems, in->mStartMenuPath, TYPE_STARTMENU);
				in->GetLnkFiles(in->mCommonStartMenuItems, in->mCommonStartMenuPath, TYPE_STARTMENU);
			}
		});
		th.detach();
		in->mIsFirstCall = false;
	}

	items.clear();

	std::lock_guard<std::mutex> lock(in->mMutex);
	if (in->mIsEnableRecent) {
		items.insert(items.end(), in->mRecentItems.begin(), in->mRecentItems.end());
	}
	if (in->mIsEnableStartMenu) {
		items.insert(items.end(), in->mStartMenuItems.begin(), in->mStartMenuItems.end());
		items.insert(items.end(), in->mCommonStartMenuItems.begin(), in->mCommonStartMenuItems.end());
	}
	return true;
}

} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace launcherapp

