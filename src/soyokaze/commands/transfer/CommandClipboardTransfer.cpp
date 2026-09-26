#include "pch.h"
#include "CommandClipboardTransfer.h"
#include "app/AppName.h"
#include "commands/transfer/CommandJSONEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace transfer {

struct CommandClipboardTransfer::PImpl
{
	// クリップボードのフォーマットID
	UINT mFormatId{0};
};

CommandClipboardTransfer::CommandClipboardTransfer() : in(new PImpl)
{
}

CommandClipboardTransfer::~CommandClipboardTransfer()
{
}

CommandClipboardTransfer* 
CommandClipboardTransfer::GetInstance()
{
	static CommandClipboardTransfer inst;
	return &inst;
}

bool CommandClipboardTransfer::Initialize()
{
	// クリップボード形式の登録
	in->mFormatId = RegisterClipboardFormat(APPNAME);
	if (in->mFormatId == 0) {
		spdlog::error("Failed to register clipboard format. err:{}", GetLastError());
		return false;
	}
	return in->mFormatId != 0;
}

CommandEntryIF* CommandClipboardTransfer::NewEntry(LPCTSTR cmdName)
{
	auto newEntry = new CommandJSONEntry(cmdName);
	return newEntry;
}

// クリップボードにデータを書き込む
// Send後はentryをReleaseする
bool CommandClipboardTransfer::SendEntry(CommandEntryIF* entry)
{
	if (entry == nullptr) {
		spdlog::error("CommandClipboardTransfer::SendEntry received null entry.");
		return false;
	}
	std::vector<CommandEntryIF*> entries{ entry };
	bool result = SendEntries(entries);
	entry->Release();
	return result;
}

bool CommandClipboardTransfer::SendEntries(const std::vector<CommandEntryIF*>& entries)
{
	if (in->mFormatId == 0) {
		spdlog::error("Failed to send clipboard data because the clipboard format is not initialized.");
		return false;
	}

	std::vector<uint8_t> data;
	if (CommandJSONEntry::PackEntries(entries, data) == false) {
		spdlog::error("Failed to serialize command entries for clipboard.");
		return false;
	}

	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, data.size());
	if (hMem == nullptr) {
		spdlog::error("Failed to allocate clipboard memory. err:{}", GetLastError());
		return false;
	}

	void* p = GlobalLock(hMem);
	if (p == nullptr) {
		DWORD err = GetLastError();
		GlobalFree(hMem);
		spdlog::error("Failed to lock clipboard memory. err:{}", err);
		return false;
	}
	memcpy(p, data.data(), data.size());
	GlobalUnlock(hMem);

	if (OpenClipboard(nullptr) == FALSE) {
		DWORD err = GetLastError();
		GlobalFree(hMem);
		spdlog::error("Failed to open clipboard for writing. err:{}", err);
		return false;
	}

	if (EmptyClipboard() == FALSE) {
		DWORD err = GetLastError();
		CloseClipboard();
		GlobalFree(hMem);
		spdlog::error("Failed to empty clipboard. err:{}", err);
		return false;
	}

	if (SetClipboardData(in->mFormatId, hMem) == nullptr) {
		DWORD err = GetLastError();
		CloseClipboard();
		GlobalFree(hMem);
		spdlog::error("Failed to set clipboard data. err:{}", err);
		return false;
	}

	if (CloseClipboard() == FALSE) {
		spdlog::error("Failed to close clipboard after writing. err:{}", GetLastError());
		return false;
	}
	return true;
}

// クリップボードからデータを読み、entryを生成する
// entryを解放するのは呼び出し側の責務
bool CommandClipboardTransfer::ReceiveEntry(CommandEntryIF** entry)
{
	if (entry == nullptr) {
		spdlog::error("CommandClipboardTransfer::ReceiveEntry received null output pointer.");
		return false;
	}
	*entry = nullptr;
	std::vector<RefPtr<CommandEntryIF>> entries;
	if (ReceiveEntries(entries) == false) {
		return false;
	}
	if (entries.empty()) {
		spdlog::error("No command entries were received from clipboard.");
		return false;
	}
	*entry = entries[0].release();
	return true;
}

bool CommandClipboardTransfer::ReceiveEntries(std::vector<RefPtr<CommandEntryIF>>& entries)
{
	entries.clear();
	if (in->mFormatId == 0) {
		spdlog::error("Failed to receive clipboard data because the clipboard format is not initialized.");
		return false;
	}

	if (OpenClipboard(nullptr) == FALSE) {
		spdlog::error("Failed to open clipboard for reading. err:{}", GetLastError());
		return false;
	}

	auto hMem = GetClipboardData(in->mFormatId);
	if (hMem == nullptr) {
		DWORD err = GetLastError();
		CloseClipboard();
		spdlog::error("Failed to get command data from clipboard. err:{}", err);
		return false;
	}

	size_t size = GlobalSize(hMem);
	if (size == 0) {
		DWORD err = GetLastError();
		CloseClipboard();
		spdlog::error("Clipboard data has invalid size. err:{}", err);
		return false;
	}

	void* p = GlobalLock(hMem);
	if (p == nullptr) {
		DWORD err = GetLastError();
		CloseClipboard();
		spdlog::error("Failed to lock clipboard data. err:{}", err);
		return false;
	}

	std::vector<uint8_t> data(size);
	memcpy(data.data(), p, data.size());
	GlobalUnlock(hMem);

	if (CloseClipboard() == FALSE) {
		spdlog::error("Failed to close clipboard after reading. err:{}", GetLastError());
		return false;
	}

	if (CommandJSONEntry::UnpackEntries(data, entries) == false) {
		spdlog::error("Failed to deserialize command entries from clipboard.");
		return false;
	}
	return true;
}


}}}

