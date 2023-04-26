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

	Command* Query(const CString& strQueryStr);
	void Query(const CString& strQueryStr, std::vector<Command*>& items);

protected:
	struct PImpl;
	PImpl* in;
};

