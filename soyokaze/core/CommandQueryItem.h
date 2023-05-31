#pragma once

#include "CommandIF.h"

class CommandQueryItem
{
public:
	CommandQueryItem(int level, soyokaze::core::Command* cmd);
	CommandQueryItem(const CommandQueryItem&) = default;

	int mMatchLevel;
	soyokaze::core::Command* mCommand;
};
