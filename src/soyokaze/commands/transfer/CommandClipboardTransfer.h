#pragma once

#include <memory>

#include "commands/core/CommandEntryIF.h"

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

	bool ReceiveEntry(CommandEntryIF** entry);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}}

