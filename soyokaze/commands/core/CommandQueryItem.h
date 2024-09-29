#pragma once

#include "commands/core/CommandIF.h"
#include <memory>
#include <vector>
#include <functional>

namespace launcherapp {

class CommandQueryItem
{
	using Command = launcherapp::core::Command;
public:
	CommandQueryItem(int level, Command* cmd);
	CommandQueryItem(const CommandQueryItem&);
	~CommandQueryItem();

	CommandQueryItem& operator = (const CommandQueryItem&);

	int mMatchLevel;
	std::unique_ptr<Command, std::function<void(void*)> > mCommand;
};

class CommandQueryItemList
{
	using Command = launcherapp::core::Command;
public:
	CommandQueryItemList();
	~CommandQueryItemList();

	bool IsEmpty() const;

	bool FindWholeMatchItem(Command** item);

	void Add(const CommandQueryItem& item);
	size_t GetItemCount();
	size_t GetItems(Command** array, size_t arrayLen);

	void Sort();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}

