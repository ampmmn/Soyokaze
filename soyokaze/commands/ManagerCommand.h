#pragma once

#include "CommandIF.h"

class CommandRepository;

class ManagerCommand : public Command
{
public:
	ManagerCommand(CommandRepository* cmdMapPtr);
	virtual ~ManagerCommand();

	virtual CString GetName();
	virtual CString GetDescription();
	virtual BOOL Execute();
	virtual BOOL Execute(const std::vector<CString>& args);
	virtual CString GetErrorString();
	virtual HICON GetIcon();
	virtual int Match(Pattern* pattern);
	virtual Command* Clone();

protected:
	CommandRepository* mCmdMapPtr;
};

