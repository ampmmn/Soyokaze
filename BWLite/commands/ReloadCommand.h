#pragma once

#include "CommandIF.h"

class CommandMap;

class ReloadCommand : public Command
{
public:
	ReloadCommand(CommandMap* pMap);
	virtual ~ReloadCommand();

	virtual CString GetName();
	virtual CString GetDescription();

	virtual BOOL Execute();
	virtual CString GetErrorString();

protected:
	CommandMap* m_pCommandMap;
};
