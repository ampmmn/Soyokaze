#pragma once

#include "CommandIF.h"

class CommandRepository;

class RegistWinCommand : public Command
{
public:
	RegistWinCommand(CommandRepository* cmdMapPtr);
	virtual ~RegistWinCommand();

	virtual CString GetName();
	virtual CString GetDescription();
	virtual BOOL Execute();
	virtual BOOL Execute(const std::vector<CString>& args);
	virtual CString GetErrorString();
	virtual HICON GetIcon();
	virtual int Match(Pattern* pattern);
	virtual Command* Clone();

protected:
	struct PImpl;
	PImpl* in;
};

