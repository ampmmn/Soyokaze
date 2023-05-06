#pragma once

#include "CommandIF.h"

class CommandMap;

class EditCommand : public Command
{
public:
	EditCommand(CommandMap* cmdMapPtr);
	virtual ~EditCommand();

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

