#include "pch.h"
#include "PathWatcher.h"
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include "commands/common/Message.h"

namespace soyokaze {
namespace commands {
namespace watchpath {

struct WATCH_TARGET
{
	WATCH_TARGET() : mDirHandle(nullptr)
	{
		OVERLAPPED olp = {};
		mOverlapped = olp;
	}
	HANDLE mDirHandle;
	OVERLAPPED mOverlapped;
	CString mPath;
	std::vector<BYTE> mBuffer;
};

struct PathWatcher::PImpl
{
	void StartWatch();
	void Abort() {
		std::lock_guard<std::mutex> lock(mMutex);
		mIsAbort = true;
	}
	bool IsAbort() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsAbort;
	}
	void GetTagetObjects(std::vector<HANDLE>& objects);
	void NotifyTarget(HANDLE event);

	std::mutex mMutex;
	std::map<CString, WATCH_TARGET> mTargets;
	bool mIsAbort;
	std::unique_ptr<std::thread> mTask;
};

void PathWatcher::PImpl::StartWatch()
{
	std::lock_guard<std::mutex> lock(mMutex);
	if (mTask.get()) {
		return;
	}

	mTask.reset(new std::thread([&]() {

		std::vector<HANDLE> objects;
		while(IsAbort() == false) {
			GetTagetObjects(objects);	
			if (objects.empty()) {
				Sleep(250);
				continue;
			}

			DWORD count = (DWORD)objects.size();
			DWORD n = WaitForMultipleObjects(count, &objects.front(), FALSE, 250);
			if (n == WAIT_TIMEOUT || n == WAIT_FAILED) {
				continue;
			}
			if (WAIT_ABANDONED_0 <= n && n < WAIT_ABANDONED_0 + count) {
				continue;
			}
			
			// 変更を通知する
			int index = (int)(n - WAIT_OBJECT_0);
			NotifyTarget(objects[index]);
		}

		for (auto& item : mTargets) {
			auto& target = item.second;
			if (target.mDirHandle) {
				CancelIo(target.mDirHandle);
				WaitForSingleObject(target.mOverlapped.hEvent, INFINITE);
			}
		}

	}));
	mTask->detach();
}


void PathWatcher::PImpl::GetTagetObjects(std::vector<HANDLE>& objects)
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

			CString notifyMsg;
			notifyMsg.Format(_T("【%s】更新を検知 : %s"), item.first, target.mPath);

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
			soyokaze::commands::common::PopupMessage(notifyMsg);

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
}

PathWatcher::~PathWatcher()
{
	in->Abort();
	Sleep(300);
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
			return;
		}

		WATCH_TARGET item;
		item.mPath = path;
		in->mTargets[cmdName] = item;
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

