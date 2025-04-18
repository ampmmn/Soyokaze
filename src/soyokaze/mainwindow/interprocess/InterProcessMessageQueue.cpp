#include "pch.h"
#include "InterProcessMessageQueue.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace mainwindow {
namespace interprocess {

constexpr LPCTSTR MAPNAME = _T("launcher_sharedmemory_command");
constexpr LPCTSTR SYNC_NAME = _T("launcher_interprocess_read");

struct ENTRY {
	int32_t mID{0};    // イベントID
	int32_t mDataLen{0};  // データ長
  char* mData[1];    // データ領域の先頭
};

struct InterProcessMessageQueue::PImpl
{
};

InterProcessMessageQueue::InterProcessMessageQueue() : in(new PImpl)
{
}

InterProcessMessageQueue::~InterProcessMessageQueue()
{
}

InterProcessMessageQueue* InterProcessMessageQueue::GetInstance()
{
	static InterProcessMessageQueue inst;
	return &inst;	
}


bool InterProcessMessageQueue::Enqueue(EVENT_ID id, void* data, size_t len)
{
	// 共有メモリを作成する
	size_t size = sizeof(int32_t) + sizeof(int32_t) + len;
	HANDLE hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD)size, MAPNAME);
	if (hMapFile == nullptr) {
		spdlog::error(_T("Failed to open memory {}"), (LPCTSTR)MAPNAME);
		return false;
	}

	// 共有メモリ領域に書きこむ
	ENTRY* entry = (ENTRY*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, size);
	if (entry == nullptr) {
		spdlog::error(_T("Failed to mapfile"));
		return false;
	}
	entry->mID = id;
	entry->mDataLen = (int32_t)len;
	memcpy(entry->mData, data, len);

	// 親側が読み取るのを待つ
	CEvent evt(FALSE, FALSE, SYNC_NAME);
	if (WaitForSingleObject(evt, 5000) != WAIT_OBJECT_0) {
		SPDLOG_WARN("Timeout occured.");
	}

	// 後始末
	if (entry) {
		UnmapViewOfFile(entry);
	}
	if (hMapFile) {
		CloseHandle(hMapFile);
	}
	return true;
}

bool InterProcessMessageQueue::Dequeue(EVENT_ID* id, std::vector<uint8_t>& stm)
{
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, MAPNAME);
	if (hMapFile == nullptr) {
		// ない
		return false;
	}

	// まずIDとデータ長サイズを読む
	int32_t* head = (int32_t*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int32_t) * 2);
	if (head == nullptr) {
		CloseHandle(hMapFile);
		return false;
	}

	*id = (EVENT_ID)head[0];
	int32_t dataLen = head[1];
	stm.resize(dataLen);

	// データを読む
	if (dataLen > 0) {
		uint8_t* data = (uint8_t*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int32_t) * 2 + dataLen);
		if (data) {
			memcpy(stm.data(), data + sizeof(int32_t) * 2, dataLen);
			UnmapViewOfFile(data);
		}
	}

	// 後始末
	if (head) {
		UnmapViewOfFile(head);
	}
	if (hMapFile) {
		CloseHandle(hMapFile);
	}

	// 読み取った旨をイベント経由で通知する
	CEvent evt(FALSE, FALSE, SYNC_NAME);
	evt.SetEvent();

	return true;
}

}
}
}

