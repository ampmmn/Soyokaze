#pragma once

#include "CommandIF.h"

class ExitCommand : public Command
{
public:
	ExitCommand();
	virtual ~ExitCommand();

	virtual CString GetName();
	virtual CString GetDescription();
	virtual BOOL Execute();
	virtual CString GetErrorString();
	virtual BOOL Match(const CString& strQueryStr);
};
