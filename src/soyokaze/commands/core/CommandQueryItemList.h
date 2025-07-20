#pragma once

#include "commands/core/CommandIF.h"
#include "commands/core/CommandQueryItem.h"
#include <memory>

namespace launcherapp {

class CommandQueryItemList
{
	using Command = launcherapp::core::Command;
public:
	CommandQueryItemList();
	~CommandQueryItemList();

	bool FindWholeMatchItem(Command** item);

	void Add(const CommandQueryItem& item);
	size_t GetItemCount();
	bool GetItem(size_t index, CommandQueryItem* item);

	void Sort();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}

