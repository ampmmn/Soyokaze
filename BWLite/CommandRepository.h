#pragma once

#include "CommandIF.h"
#include <vector>

class CommandRepository
{
public:
	CommandRepository();
	~CommandRepository();

public:
	BOOL Load();

	int NewCommandDialog(const CString* cmdNamePtr);
	int EditCommandDialog(const CString& cmdName);
	int ManagerDialog();

	bool DeleteCommand(const CString& cmdName);

	void EnumCommands(std::vector<Command*>& commands);

	bool IsBuiltinName(const CString& cmdName);

	void Query(const CString& strQueryStr, std::vector<Command*>& commands);
	Command* QueryAsWholeMatch(const CString& strQueryStr);

	bool IsValidAsName(const CString& strQueryStr);

protected:
	struct PImpl;
	PImpl* in;
};

