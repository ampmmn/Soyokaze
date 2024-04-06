#include "pch.h"
#include "PathWatcher.h"
#include "utility/SHA1.h"
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <deque>
#include "commands/common/Message.h"
#include "commands/watchpath/WatchPathToast.h"
#include "settingwindow/ShortcutSettingPage.h"

namespace soyokaze {
namespace commands {
namespace watchpath {

// 監視対象パスについての管理情報
struct WATCH_TARGET
{
	WATCH_TARGET() : mDirHandle(nullptr)
	{
		// 初期化
		OVERLAPPED olp = {};
		mOverlapped = olp;
	}

	// 監視対象パス
	CString mPath;
	// ReadDirectoryChangesWに渡すためのファイルハンドル
	HANDLE mDirHandle;
	// 更新検知を受け取るためのイベントを含むOVERLAPPED構造体
	OVERLAPPED mOverlapped;
	// 変更通知受信用のバッファ
	std::vector<BYTE> mBuffer;
};

using TimeStampList = std::vector<std::pair<CString, FILETIME> >;

static bool GetLastUpdateTime(LPCTSTR path, FILETIME& ftime)
{
	HANDLE h = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}
	GetFileTime(h, nullptr, nullptr, &ftime);
	CloseHandle(h);
	return true;
}

// 監視対象パスについての管理情報(ネットワークパス用)
struct WATCH_TARGET_UNC
{
	WATCH_TARGET_UNC()
	{
	}

	bool CreateTimeStamp(TimeStampList& fileList) const {

		fileList.clear();

		if (PathIsDirectory(mPath) == FALSE) {
			return false;
		}

		// フォルダ内のファイル列挙
		std::deque<CString> stk;
		stk.push_back(mPath);

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
				CString filePath = f.GetFilePath();

				SHA1 sha;
				sha.Add(filePath);

				std::pair<CString, FILETIME> item;
				item.first = sha.Finish();
				GetLastUpdateTime(filePath, item.second);

				fileList.push_back(item);
			}
			f.Close();
		}

		std::sort(fileList.begin(), fileList.end(), [](const std::pair<CString, FILETIME>& l, const std::pair<CString, FILETIME>& r) {
				return l.first.CompareNoCase(r.first) < 0;
		});
		return true;
	}
	bool IsDiffer(const TimeStampList& fileList) const {

		// 要素数が異なる==差異がある
		if (mTimestamps.size() != fileList.size()) {
			return true;
		}

		size_t itemCount = fileList.size();
		for (size_t i = 0; i < itemCount; ++i) {

			const auto& itemL = fileList[i];
			const auto& itemR = mTimestamps[i];

			if (itemL.first != itemR.first) {
				// 含まれる要素に差異あり
				return true;
			}

			if (memcmp(&itemL.second, &itemR.second, sizeof(FILETIME)) != 0) {
				// 更新日時に差異あり
				return true;
			}
		}
		return false;
	}

	// 監視対象パス
	CString mPath;
	// 監視対象パス以下にある要素の更新日時情報
	TimeStampList mTimestamps;
	// 初回実施フラグ
	bool mIsFirst = true;
	// 最後にチェックした時刻
	DWORD mLastChecked = 0;
};

struct PathWatcher::PImpl
{
	// 監視を開始
	void StartWatch();

	// 監視を中止する
	void Abort() {
		std::lock_guard<std::mutex> lock(mMutex);
		mIsAbort = true;
	}
	// 監視スレッドの完了を待機する(最大3秒)
	void WaitExit() {
		DWORD start = GetTickCount();
		while (GetTickCount() - start < 3000) {
			if (mIsExited) {
				break;
			}
			Sleep(50);
		}
	}

	bool IsAbort() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsAbort;
	}
	bool GetTagetObjects(std::vector<HANDLE>& objects);
	void NotifyTarget(HANDLE event);

	bool WatchForLocalPath();
	bool WatchForUNCPath();

	void NotifyPath(const CString& cmdName, const CString& path);

	// 排他制御用
	std::mutex mMutex;

	// 監視対象
	std::map<CString, WATCH_TARGET> mTargets;
	std::map<CString, WATCH_TARGET_UNC> mUncTargets;

	// 監視終了フラグ
	bool mIsAbort;
	// 監視スレッド終了済を表すフラグ
	bool mIsExited;
	// 監視スレッド
	std::unique_ptr<std::thread> mTask;
};

// 監視を開始
void PathWatcher::PImpl::StartWatch()
{
	std::lock_guard<std::mutex> lock(mMutex);

	// タスクが作成済なら何もしない
	if (mTask.get()) {
		return;
	}

	// 初回にタスクを生成する
	mTask.reset(new std::thread([&]() {

		while(IsAbort() == false) {

			bool hasLocalItem = WatchForLocalPath();
			bool hasUNCItem = WatchForUNCPath();
			if (hasLocalItem == false && hasUNCItem == false) {
				// 監視対象がなければ5秒待機
				Sleep(5000);
				continue;
			}

		}

		// 終了処理
		for (auto& item : mTargets) {
			auto& target = item.second;
			if (target.mDirHandle) {
				CancelIo(target.mDirHandle);
				WaitForSingleObject(target.mOverlapped.hEvent, INFINITE);
			}
		}
		mIsExited = true;

	}));
	mTask->detach();
}

// ローカルパス向けの更新検知処理
bool PathWatcher::PImpl::WatchForLocalPath()
{
	std::vector<HANDLE> objects;
	if (GetTagetObjects(objects) == false) {
		return false;
	}

	// (ReadDirectoryChangesWの)更新通知があったかどうかを待つ
	DWORD count = (DWORD)objects.size();
	DWORD n = WaitForMultipleObjects(count, &objects.front(), FALSE, 1000);
	if (n == WAIT_TIMEOUT || n == WAIT_FAILED) {
		// 更新なし
		return true;
	}
	if (WAIT_ABANDONED_0 <= n && n < WAIT_ABANDONED_0 + count) {
		// 範囲外(ここに来ることは通常ないが..)
		return true;
	}

	// 変更を通知する
	int index = (int)(n - WAIT_OBJECT_0);
	NotifyTarget(objects[index]);
	return true;
}

// UNCパス向けの更新検知処理
bool PathWatcher::PImpl::WatchForUNCPath()
{
	Sleep(50);

	if (mUncTargets.empty()) {
		return false;
	}

	bool hasItem = false;

	DWORD curTime = GetTickCount();

	for (auto& pa : mUncTargets) {
		auto& cmdName = pa.first;
		auto& item = pa.second;

		// 前回に比較したときから一定時間経過してない場合はチェックしない
		// (負荷対策)
		if (item.mLastChecked != 0 && curTime - item.mLastChecked  < 60 * 1000) { // 1分に1回
			continue;
		}

		// 現在の情報を取得
		TimeStampList current;
		if (item.CreateTimeStamp(current) == false) {
			continue;
		}
		hasItem = true;

		// 初回は新規作成なので通知しない
		if (item.mIsFirst)  {
			item.mTimestamps.swap(current);
			item.mIsFirst = false;
			item.mLastChecked = curTime;
			continue;
		}

		// 前回と比較
		bool isUpdated = item.IsDiffer(current); 
		if (isUpdated == false) {
			continue;
		}

		// 通知
		NotifyPath(cmdName, item.mPath);

		item.mTimestamps.swap(current);
		item.mLastChecked = curTime;
	}

	return hasItem;
}

void PathWatcher::PImpl::NotifyPath(const CString& cmdName, const CString& path)
{
	if (ShortcutSettingPage::IsStartMenuExists()) {
		// スタートメニューにショートカットが登録されている場合はトーストを使う
		Toast toast;
		toast.SetCommandName(cmdName);
		toast.SetPath(path);
		toast.Show();
	}
	else {
		// 登録されていない場合はShell_NotifyIconのメッセージで代替する
		CString notifyMsg;
		notifyMsg.Format(_T("【%s】更新を検知 : %s"), cmdName, path);
		soyokaze::commands::common::PopupMessage(notifyMsg);
	}
}

bool PathWatcher::PImpl::GetTagetObjects(std::vector<HANDLE>& objects)
{
	objects.clear();

	std::lock_guard<std::mutex> lock(mMutex);
	for (auto& item : mTargets) {
		WATCH_TARGET& target = item.second;
		if (target.mDirHandle) {
			objects.push_back(target.mOverlapped.hEvent);
			continue;
		}

		if (PathIsDirectory(target.mPath) == FALSE) {
			continue;
		}

		// CreateFileで監視対象(フォルダ)のhandleを生成する
		HANDLE h = CreateFile(target.mPath, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		           NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
		if (h == INVALID_HANDLE_VALUE) {
			continue;
		}

		// 変更通知受信用のバッファを作成する
		target.mBuffer.resize(sizeof(FILE_NOTIFY_INFORMATION) + sizeof(TCHAR) * MAX_PATH_NTFS);

		// 更新検知するためのイベントを作成する
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		target.mOverlapped.hEvent = hEvent;

		DWORD flags = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE;

		// ReadDirectoryChangesWをコールして監視を開始する
		ResetEvent(target.mOverlapped.hEvent);
		BOOL isOK = ReadDirectoryChangesW(h, &target.mBuffer.front(), (DWORD)target.mBuffer.size(), TRUE, flags, nullptr, &target.mOverlapped, nullptr);
		if (isOK == FALSE) {
			CloseHandle(h);
			CloseHandle(hEvent);
			continue;
		}
		target.mDirHandle = h;

		objects.push_back(target.mOverlapped.hEvent);
	}

	return objects.size() > 0;
}

void PathWatcher::PImpl::NotifyTarget(HANDLE event)
{
	// 通知されたイベントに対応する要素を探す
	for (auto& item : mTargets) {
		auto& target = item.second;
		if (target.mOverlapped.hEvent != event) {
			continue;
		}

		DWORD size = 0;
		BOOL isOK = GetOverlappedResult(target.mDirHandle, &target.mOverlapped, &size, FALSE);
		ResetEvent(target.mOverlapped.hEvent);
		if (isOK) {
		// 変更通知情報を受け取り、通知する
			FILE_NOTIFY_INFORMATION* data = (FILE_NOTIFY_INFORMATION*)&target.mBuffer.front();

			for (;;) {

				int action = data->Action;
				// FILE_ACTION_ADDED or FILE_ACTION_REMOVED or FILE_ACTION_MODIFIED or FILE_ACTION_RENAMED_OLD_NAME or FILE_ACTION_RENAMED_NEW_NAME

				DWORD lenInByte = data->FileNameLength;
				std::vector<TCHAR> fileName(lenInByte / sizeof(TCHAR) + 1);
				memcpy(&fileName.front(), data->FileName, lenInByte);

				if (data->NextEntryOffset == 0) {
					break;
				}
				data = (FILE_NOTIFY_INFORMATION*)((LPBYTE)(data) + data->NextEntryOffset);
			}

			// 通知
			NotifyPath(item.first, target.mPath);

			// フォルダがなくなっていたら終了
			if (PathIsDirectory(target.mPath) == FALSE) {
				CloseHandle(target.mDirHandle);
				target.mDirHandle = nullptr;
				CloseHandle(target.mOverlapped.hEvent);
				target.mOverlapped.hEvent = nullptr;
				return;
			}

			// 再度ReadDirectoryChanges
			DWORD flags = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE;
			ResetEvent(target.mOverlapped.hEvent);
			BOOL isOK = ReadDirectoryChangesW(target.mDirHandle, &target.mBuffer.front(), (DWORD)target.mBuffer.size(), TRUE, flags, nullptr, &target.mOverlapped, nullptr);
		}
	}
}

PathWatcher::PathWatcher() : in(new PImpl)
{
	in->mIsAbort = false;
	in->mIsExited = false;
}

PathWatcher::~PathWatcher()
{
	in->Abort();
	in->WaitExit();
}

PathWatcher* PathWatcher::Get()
{
	static PathWatcher inst;
	return &inst;
}

void PathWatcher::RegisterPath(const CString& cmdName, const CString& path)
{
	{
		std::lock_guard<std::mutex> lock(in->mMutex);

		auto it = in->mTargets.find(cmdName);
		if (it != in->mTargets.end()) {
			// 同じコマンドですでに登録済
			return;
		}

		if (PathIsUNC(path) == FALSE) {
			WATCH_TARGET item;
			item.mPath = path;
			in->mTargets[cmdName] = item;
		}
		else {
			WATCH_TARGET_UNC item;
			item.mPath = path;
			in->mUncTargets[cmdName] = item;
		}
	}
	in->StartWatch();
}

void PathWatcher::UnregisterPath(const CString& cmdName)
{
	std::lock_guard<std::mutex> lock(in->mMutex);

	auto it = in->mTargets.find(cmdName);
	if (it == in->mTargets.end()) {
		return;
	}
	in->mTargets.erase(it);
}

}
}
}

