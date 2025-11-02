#include "pch.h"
#include "LocalDirectoryWatcher.h"
#include "utility/Path.h"
#include <atomic>
#include <map>
#include <vector>
#include <mutex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct LocalDirectoryWatcher::Element
{
	~Element()
	{
		if (mDirHandle) {
			CloseHandle(mDirHandle);
		}
		if (mOverlapped.hEvent) {
			CloseHandle(mOverlapped.hEvent);
		}
	}
	// 監視対象ディレクトリ
	CString mDirPath;
	// 監視ファイル(空の場合はディレクトリの変更を検知)
	CString mFileName;
	// 監視ディレクトリハンドル
	HANDLE mDirHandle{nullptr};
	// 非同期
	OVERLAPPED mOverlapped{};
	// コールバック関数
	LPCALLBACKFUNC mCallback{nullptr};
	void* mParam{nullptr};
	// 変更情報を受け取るためのバッファ
	std::vector<uint8_t> mBuffer;
	bool mIsDelete{false};
};

struct LocalDirectoryWatcher::PImpl
{
	void Abort()
	{
		mIsAbort = true;
	}
	bool IsAbort()
	{
		return mIsAbort.load();
	}

	void StartWatch()
	{
		HRESULT hr = CoInitialize(NULL);
		if (FAILED(hr)) {
			spdlog::error("Failed to initialize COM library. hr:{}", hr);
		}

		while(IsAbort() == false) {

			// 監視対象のイベント一覧を取得する
			std::vector<HANDLE> events;
			GetEvents(events);

			auto count = (int)events.size();
			if (count == 0) {
				// 監視対象イベントなし
				Sleep(50);
				continue;
			}

			// イベントを待つ
			auto n = WaitForMultipleObjects((DWORD)count, events.data(), FALSE, 250);
			if (n < WAIT_OBJECT_0 || WAIT_OBJECT_0 + count <= n) {
				// 何もなし
			 continue;
			}	 

			// イベントを通知する
			int index = n - WAIT_OBJECT_0;
			Notify(events[index]);
		}
		CoUninitialize();
	}

	// 監視を開始する
	void CallReadDirectoryChanges(Element* elem)
	{
		// 変更検知の対象とする対象のビットフラグ
		DWORD flags = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
		             	FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE;

		// 監視を開始
		ResetEvent(elem->mOverlapped.hEvent);
		BOOL isOK = ReadDirectoryChangesW(elem->mDirHandle, &elem->mBuffer.front(), (DWORD)elem->mBuffer.size(),
		                                  TRUE, flags, nullptr, &elem->mOverlapped, nullptr);
		if (isOK == FALSE) {
			spdlog::warn(_T("failed to call ReadDirectoryChangesW. path:{}"), (LPCTSTR)elem->mDirPath);
		}
	}

	void GetEvents(std::vector<HANDLE>& events)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		// 削除フラグが立っている要素があったら破棄する
		for (auto it = mIdToElementMap.begin(); it != mIdToElementMap.end();) {
			auto& elem = it->second;
			if (elem->mIsDelete) {
				it = mIdToElementMap.erase(it);
				continue;
			}
			else {
				it++;
			}
		}

		// 残りの要素一覧からイベントのリストを作成する
		for (auto& item : mIdToElementMap) {

			auto& elem = item.second;
			if (elem->mBuffer.empty() == false) {
				// 2回目からは作成済のイベントハンドルを返す
				events.push_back(elem->mOverlapped.hEvent);
				continue;
			}

			// 初回は変更通知受信用のバッファを作成し、監視を開始する
			elem->mBuffer.resize(sizeof(FILE_NOTIFY_INFORMATION) + sizeof(TCHAR) * MAX_PATH_NTFS);
			CallReadDirectoryChanges(elem.get());

			// イベントハンドルを追加
			events.push_back(elem->mOverlapped.hEvent);
		}

	}

	void Notify(HANDLE hEvent)
	{
		// 対象イベントの状態をもとに戻す
		ResetEvent(hEvent);

		std::lock_guard<std::mutex> lock(mMutex);
		for (auto& item : mIdToElementMap) {

			// 対象イベントハンドルから対象要素を特定する
			auto& elem = item.second;
			if (hEvent != elem->mOverlapped.hEvent) {
				continue;
			}

			// 
			DWORD size = 0;
			BOOL isOK = GetOverlappedResult(elem->mDirHandle, &elem->mOverlapped, &size, FALSE);
			if (isOK == FALSE) {
				spdlog::warn(_T("Failed to GetOverlappedResult path:{}"), (LPCTSTR)elem->mDirPath);
				return ;
			}

			// 変更通知情報を受け取る
			FILE_NOTIFY_INFORMATION* data = (FILE_NOTIFY_INFORMATION*)&elem->mBuffer.front();

			if (IsFileContained(elem->mFileName, data) == false) {
				CallReadDirectoryChanges(elem.get());
				return;
			}

			// 通知する
			elem->mCallback(elem->mParam);
			// 次の監視を開始する
			CallReadDirectoryChanges(elem.get());
			return;
		}
	}

	// 変更通知情報のなかに所定のファイル名が含まれているか?
	bool IsFileContained(const CString& targetName, FILE_NOTIFY_INFORMATION* data)
	{
		if (targetName.IsEmpty()) {
			// ファイル名が空の場合は含まれてるものとする
			return true;
		}

		tstring fileName;
		for (;;) {

			int action = data->Action;
			// FILE_ACTION_ADDED or FILE_ACTION_REMOVED or FILE_ACTION_MODIFIED or
			// FILE_ACTION_RENAMED_OLD_NAME or FILE_ACTION_RENAMED_NEW_NAME

			DWORD lenInByte = data->FileNameLength;
			size_t lenInStrCount = lenInByte / sizeof(TCHAR);

			// 変更通知情報からファイル名の部分を取り出す
			fileName.assign(data->FileName, lenInStrCount);

			// 一致するかどうか
			if ((action != FILE_ACTION_RENAMED_OLD_NAME) && targetName == fileName.c_str()) {
				return true;
			}

			if (data->NextEntryOffset == 0) {
				break;
			}
			data = (FILE_NOTIFY_INFORMATION*)((LPBYTE)(data) + data->NextEntryOffset);
		}
		return false;
	}

	// ファイル監視登録者の一覧
	std::map<uint32_t, std::unique_ptr<Element> > mIdToElementMap;
	// 次のID番号
	uint32_t mNextId{1};
	// 終了フラグ
	std::atomic<bool> mIsAbort{false};
	// 排他制御
	std::mutex mMutex;
	// 監視スレッドオブジェクト
	std::thread mWatchThread;
};

LocalDirectoryWatcher::LocalDirectoryWatcher() : in(new PImpl)
{
}

LocalDirectoryWatcher::~LocalDirectoryWatcher()
{
}

LocalDirectoryWatcher* LocalDirectoryWatcher::GetInstance()
{
	static LocalDirectoryWatcher inst;
	return &inst;
}

void LocalDirectoryWatcher::Finalize()
{
	// 終了フラグを立ててスレッド完了を待つ
	if (in->mWatchThread.joinable()) {
		in->Abort();
		in->mWatchThread.join();
	}

	// 各種オブジェクトをすべてクリアする
	in->mIdToElementMap.clear();
}

// 登録
uint32_t LocalDirectoryWatcher::Register(LPCTSTR pathToDir, LPCALLBACKFUNC func, void* param)
{
	Path path(pathToDir);

	// ファイルパスだったら末尾の要素をカットしてディレクトリパスにする
	CString fileName;
	if (path.FileExists() && path.IsDirectory() == false) {
		fileName = path.FindFileName();
		path.RemoveFileSpec();
	}
	// カットした後、有効なディレクトリパスでなかったらエラーとする
	if (path.IsDirectory() == false) {
		return 0;
	}

	// CreateFileで監視対象(フォルダ)のhandleを生成する
	HANDLE h = CreateFile(path, FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		spdlog::warn(_T("path does not exist. {}"), (LPCTSTR)path);
		return 0;
	}

	// 更新検知するためのイベントを作成する
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == nullptr) {
		spdlog::error(_T("Failed to CreateEvent!"));
		CloseHandle(h);
		return false;
	}

	std::lock_guard<std::mutex> lock(in->mMutex);
	// 初回呼び出し時に監視スレッドを作成する
	if (in->mIdToElementMap.empty()) {
		std::thread th([&]() { in->StartWatch(); });
		in->mWatchThread.swap(th);
	}

	auto id = in->mNextId++;
	auto elem = new Element{ path, fileName, h, {0, 0, {}, hEvent}, func, param };
	in->mIdToElementMap[id] = std::unique_ptr<Element>(elem);
	return id;
}

// 登録を解除
bool LocalDirectoryWatcher::Unregister(uint32_t id)
{
	std::lock_guard<std::mutex> lock(in->mMutex);

	auto it = in->mIdToElementMap.find(id);
	if (it == in->mIdToElementMap.end()) {
		return false;
	}

	auto& elem = it->second;
	elem->mIsDelete = true;
	return true;
}

