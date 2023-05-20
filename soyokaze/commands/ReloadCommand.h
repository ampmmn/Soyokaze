#pragma once

#include "CommandIF.h"

class CommandRepository;

class ReloadCommand : public Command
{
public:
	ReloadCommand(CommandRepository* pMap);
	virtual ~ReloadCommand();

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
