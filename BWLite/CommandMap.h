#pragma once

#include "CommandIF.h"

class CommandMap
{
public:
	CommandMap();
	~CommandMap();

public:
	BOOL Load();
	Command* Query(const CString& strQueryStr);

protected:
	struct PImpl;
	PImpl* in;
};

