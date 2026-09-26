#pragma once

#include <memory>
#include <vector>

#include "commands/core/CommandEntryIF.h"
#include "utility/RefPtr.h"

namespace launcherapp { namespace commands { namespace transfer {

class CommandClipboardTransfer
{
	CommandClipboardTransfer();
	~CommandClipboardTransfer();

public:
	static CommandClipboardTransfer* GetInstance();

	bool Initialize();

	CommandEntryIF* NewEntry(LPCTSTR cmdName);
	bool SendEntry(CommandEntryIF* entry);
	bool SendEntries(const std::vector<CommandEntryIF*>& entries);

	bool ReceiveEntry(CommandEntryIF** entry);
	bool ReceiveEntries(std::vector<RefPtr<CommandEntryIF>>& entries);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}}

