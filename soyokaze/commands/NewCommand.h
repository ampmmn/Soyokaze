#pragma once

#include "CommandIF.h"

class CommandRepository;

class NewCommand : public Command
{
public:
	NewCommand(CommandRepository* cmdMapPtr);
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
	CommandRepository* mCmdMapPtr;
};

