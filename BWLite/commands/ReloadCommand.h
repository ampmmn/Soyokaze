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
	virtual BOOL Match(const CString& strQueryStr);

protected:
	CommandMap* m_pCommandMap;
};
