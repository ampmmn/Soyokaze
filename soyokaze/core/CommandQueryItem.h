#pragma once

#include "CommandIF.h"
#include <memory>
#include <vector>
#include <functional>

namespace soyokaze {

class CommandQueryItem
{
	using Command = soyokaze::core::Command;
public:
	CommandQueryItem(int level, Command* cmd);
	CommandQueryItem(const CommandQueryItem&);
	~CommandQueryItem();

	CommandQueryItem& operator = (const CommandQueryItem&);

	int mMatchLevel;
	std::unique_ptr<Command, std::function<void(void*)> > mCommand;
};

using CommandQueryItemList = std::vector<CommandQueryItem>;

}

