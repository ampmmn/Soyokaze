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
		GetLnkFiles(tmp, CSIDL_RECENT);
		// ユーザのスタートメニュー
		GetLnkFiles(tmp, CSIDL_STARTMENU);
		// ユーザのデスクトップ
		GetFiles(tmp, CSIDL_DESKTOP);
		// すべてのユーザのスタートメニュー
		GetLnkFiles(tmp, CSIDL_COMMON_STARTMENU);
		// すべてのユーザのデスクトップ
		GetFiles(tmp, CSIDL_COMMON_DESKTOPDIRECTORY);

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

void SpecialFolderFiles::GetLnkFiles(std::vector<ITEM>& items, int csidl)
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
			item.mType = ITEM::GetTypeFromCSIDL(csidl);
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
			bool isShortcut = (_tcsicmp(PathFindExtension(fileName), _T(".lnk")) == 0);

			ITEM item;
			item.mType = ITEM::GetTypeFromCSIDL(csidl);
			item.mName = PathFindFileName(fileName);

			if (isShortcut) {
				item.mFullPath = ShortcutFile::ResolvePath(f.GetFilePath(), &item.mDescription);
			}
			else {
				item.mFullPath = f.GetFilePath();
				item.mDescription = f.GetFilePath();
			}

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

} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace soyokaze

