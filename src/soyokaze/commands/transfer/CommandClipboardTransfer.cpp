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
		return false;
	}
	std::vector<uint8_t> data;
	if (entry->DumpRawData(data) == false) {
		entry->Release();
		return false;
	}
	entry->Release();

	//FILE* fp;
	//if (fopen_s(&fp, "c:\\users\\htmny\\debug.txt", "a") == 0) {
	//	fprintf(fp, "%s\n", (const char*)data.data());
	//	fclose(fp);
	//}

	if (OpenClipboard(nullptr) == FALSE) {
		return false;
	}

	EmptyClipboard();

	auto hMem = GlobalAlloc(GMEM_MOVEABLE, data.size());
	if (hMem == nullptr) {
		CloseClipboard();
		return false;
	}

	void* p = GlobalLock(hMem);
	memcpy(p, data.data(), data.size());
	GlobalUnlock(p);

	SetClipboardData(in->mFormatId, hMem);

	CloseClipboard();

	return true;
}

// クリップボードからデータを読み、entryを生成する
// entryを解放するのは呼び出し側の責務
bool CommandClipboardTransfer::ReceiveEntry(CommandEntryIF** entry)
{
	if (entry == nullptr) {
		return false;
	}

	if (OpenClipboard(nullptr) == FALSE) {
		return false;
	}

	auto hMem = GetClipboardData(in->mFormatId);
	if (hMem == nullptr) {
		CloseClipboard();
		return false;
	}

	size_t size = GlobalSize(hMem);
	void* p = GlobalLock(hMem);

	std::vector<uint8_t> data(size);
	memcpy(data.data(), p, data.size());
	GlobalUnlock(p);

	CloseClipboard();

	*entry = new CommandJSONEntry(data);
	return true;
}


}}}

