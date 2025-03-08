#include "pch.h"
#include "SpecialFolderFiles.h"
#include "utility/ShortcutFile.h"
#include "utility/Path.h"
#include <mutex>
#include <thread>
#include <deque>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace specialfolderfiles {

static const int INTERVAL = 5000;

struct SpecialFolderFiles::PImpl
{
	PImpl() : mEvent(TRUE, TRUE)
	{}

	std::mutex mMutex;
	std::vector<ITEM> mItems;
	uint64_t mElapsed = 0;
	CEvent mEvent;

	bool mIsUpdated = false;
	bool mIsEnableRecent = true;
	bool mIsEnableStartMenu = true;
};


SpecialFolderFiles::SpecialFolderFiles() : in(std::make_unique<PImpl>())
{
}

SpecialFolderFiles::~SpecialFolderFiles()
{
	WaitForSingleObject(in->mEvent, 3000);
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
	{
		std::lock_guard<std::mutex> lock(in->mMutex);
		uint64_t elapsed = GetTickCount64() - in->mElapsed;
		if (elapsed <= INTERVAL) {
			if (in->mIsUpdated) {
				items = in->mItems;
				in->mIsUpdated = false;
				return true;
			}
			return false;
		}
	}

	in->mEvent.ResetEvent();

	std::thread th([&]() {
		
		HRESULT hr = CoInitialize(NULL);
		if (FAILED(hr)) {
			SPDLOG_ERROR(_T("Failed to CoInitialize!"));
		}

		std::vector<ITEM> tmp;

		if (in->mIsEnableRecent) {
			// 最近使ったファイルを得る
			GetLnkFiles(tmp, CSIDL_RECENT);
		}
		if (in->mIsEnableStartMenu) {
			// ユーザのスタートメニュー
			GetLnkFiles(tmp, CSIDL_STARTMENU);
			// すべてのユーザのスタートメニュー
			GetLnkFiles(tmp, CSIDL_COMMON_STARTMENU);
		}

		CoUninitialize();

		std::lock_guard<std::mutex> lock(in->mMutex);
		in->mItems.swap(tmp);
		in->mElapsed = GetTickCount64();
		in->mIsUpdated = true;
		in->mEvent.SetEvent();
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
	std::vector<TCHAR> path(MAX_PATH_NTFS);
	SHGetSpecialFolderPath(NULL, path.data(), csidl, 0);
	PathAppend(path.data(), _T("*.*"));

	// フォルダ内のファイル列挙
	std::deque<CString> stk;
	stk.push_back(path.data());

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
			item.mType = ITEM::GetTypeFromCSIDL(csidl);
			item.mName = PathFindFileName(fileName);

			PathRemoveExtension(item.mName.GetBuffer(item.mName.GetLength()));   // .lnkを抜く
			item.mName.ReleaseBuffer();
			item.mFullPath = ShortcutFile::ResolvePath(f.GetFilePath(), &item.mDescription);

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

} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace launcherapp

