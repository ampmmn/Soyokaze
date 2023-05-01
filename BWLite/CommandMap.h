#pragma once

#include "CommandIF.h"
#include <vector>

class CommandMap
{
public:
	CommandMap();
	~CommandMap();

public:
	BOOL Load();

	void Query(const CString& strQueryStr, std::vector<Command*>& items);
	Command* QueryAsWholeMatch(const CString& strQueryStr);

protected:
	struct PImpl;
	PImpl* in;
};

