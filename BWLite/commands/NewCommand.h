#pragma once

#include "CommandIF.h"

class CommandMap;

class NewCommand : public Command
{
public:
	NewCommand(CommandMap* cmdMapPtr);
	virtual ~NewCommand();

	virtual CString GetName();
	virtual CString GetDescription();
	virtual BOOL Execute();
	virtual BOOL Execute(const std::vector<CString>& args);
	virtual CString GetErrorString();
	virtual HICON GetIcon();
	virtual BOOL Match(Pattern* pattern);
	virtual Command* Clone();

protected:
	CommandMap* mCmdMapPtr;
};

