#include "pch.h"
#include "SpecialFolderFileFind.h"
#include "utility/ShortcutFile.h"
#include "utility/Path.h"
#include "utility/LocalDirectoryWatcher.h"
#include <mutex>
#include <deque>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace specialfolderfiles {

struct SpecialFolderFileFind::PImpl
{
	void RegisterWatcher();
	void GetLnkFiles(std::vector<ITEM>& items, const CString& directoryPath, int type);

	void UpdateRecentItems();
	void UpdateStartMenuItems();
	void UpdateCommonStartMenuItems();

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

void SpecialFolderFileFind::PImpl::RegisterWatcher()
{
	auto watcher = LocalDirectoryWatcher::GetInstance();

	Path path;

	// 最近使ったファイルのフォルダが更新されたら通知を受け取るための登録をする
	SHGetSpecialFolderPath(NULL, (LPTSTR)path, CSIDL_RECENT, 0);
	mRecentPath = path;
	watcher->Register(mRecentPath, [](void* p) { ((PImpl*)p)->UpdateRecentItems(); }, this);

	// スタートメニューフォルダが更新されたら通知を受け取るための登録をする
	SHGetSpecialFolderPath(NULL, (LPTSTR)path, CSIDL_STARTMENU, 0);
	mStartMenuPath = path;
	watcher->Register(mStartMenuPath, [](void* p) { ((PImpl*)p)->UpdateStartMenuItems(); }, this);

	// すべてのユーザーのスタートメニューフォルダが更新されたら通知を受け取るための登録をする
	SHGetSpecialFolderPath(NULL, (LPTSTR)path, CSIDL_COMMON_STARTMENU, 0);
	mCommonStartMenuPath = path;
	watcher->Register(mCommonStartMenuPath, [](void* p) { ((PImpl*)p)->UpdateCommonStartMenuItems(); }, this);
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

void SpecialFolderFileFind::PImpl::UpdateRecentItems()
{
	if (mIsEnableRecent == false) {
		return;
	}
	std::vector<ITEM> items;
	GetLnkFiles(items, mRecentPath, TYPE_RECENT);

	std::lock_guard<std::mutex> lock(mMutex);
	mRecentItems.swap(items);
}

void SpecialFolderFileFind::PImpl::UpdateStartMenuItems()
{
	if (mIsEnableStartMenu == false) {
		return;
	}
	std::vector<ITEM> items;
	GetLnkFiles(items, mStartMenuPath, TYPE_STARTMENU);

	std::lock_guard<std::mutex> lock(mMutex);
	mStartMenuItems.swap(items);
}

void SpecialFolderFileFind::PImpl::UpdateCommonStartMenuItems()
{
	if (mIsEnableStartMenu == false) {
		return;
	}
	std::vector<ITEM> items;
	GetLnkFiles(items, mCommonStartMenuPath, TYPE_STARTMENU);

	std::lock_guard<std::mutex> lock(mMutex);
	mCommonStartMenuItems.swap(items);
}

void SpecialFolderFileFind::PImpl::GetLnkFiles(std::vector<ITEM>& items, const CString& directoryPath, int type)
{
	Path path(directoryPath);
	path.Append(_T("*.*"));

	// フォルダ内のファイル列挙
	std::deque<CString> stk;
	stk.push_back((LPTSTR)path);

	// ファイルパスと表示名の長さを保持するmap
	std::map<CString, int> pathNameMap;

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

			// フルパスと表示名(の長さ)の対応をmapで保持する
			auto itFind = pathNameMap.find(item.mFullPath);
			if (itFind != pathNameMap.end()) {
			 	if (item.mName.GetLength() < itFind->second) {
					// 最も短い表示名を採用する
					itFind->second = item.mName.GetLength();
				}
			}
			else {
				pathNameMap[item.mFullPath] = item.mName.GetLength();
			}
		}
		f.Close();
	}

	auto it = tmp.begin();
	while (it != tmp.end()) {
		auto& item = *it;
		auto itFind = pathNameMap.find(item.mFullPath);
		if (itFind != pathNameMap.end() && itFind->second < item.mName.GetLength()) {
			// 長い表示名のものを除外する
			// (一つのフルパスに対して最も表示名が短いものだけを表示する)
			it = tmp.erase(it);
			continue;
		}

		GetLastWriteTime(item.mFullPath, item.mWriteTime);
		it++;
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



SpecialFolderFileFind::SpecialFolderFileFind() : in(std::make_unique<PImpl>())
{
}

SpecialFolderFileFind::~SpecialFolderFileFind()
{
}

void SpecialFolderFileFind::EnableStartMenu(bool isEnable)
{
	in->mIsEnableStartMenu = isEnable;
}

void SpecialFolderFileFind::EnableRecent(bool isEnable)
{
	in->mIsEnableRecent = isEnable;
}

bool SpecialFolderFileFind::FindShortcutFiles(std::vector<ITEM>& items)
{
	if (in->mIsEnableStartMenu == false && in->mIsEnableRecent == false) {
		return false;
	}

	if (in->mIsFirstCall) {

		// 変更通知を受け取るための登録
		in->RegisterWatcher();

		// 初回はショートカット一覧を直接取得する。以降は変更通知経由で更新する
		std::thread th([&](){
			in->UpdateRecentItems();
			in->UpdateStartMenuItems();
			in->UpdateCommonStartMenuItems();
		});
		th.detach();
		in->mIsFirstCall = false;
	}

	items.clear();

	if (in->mIsEnableRecent) {
		std::lock_guard<std::mutex> lock(in->mMutex);
		items.insert(items.end(), in->mRecentItems.begin(), in->mRecentItems.end());
	}
	if (in->mIsEnableStartMenu) {
		std::lock_guard<std::mutex> lock(in->mMutex);
		items.insert(items.end(), in->mStartMenuItems.begin(), in->mStartMenuItems.end());
		items.insert(items.end(), in->mCommonStartMenuItems.begin(), in->mCommonStartMenuItems.end());
	}
	return true;
}

} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace launcherapp

