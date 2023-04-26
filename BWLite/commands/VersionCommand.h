#pragma once

#include "CommandIF.h"

class VersionCommand : public Command
{
public:
	VersionCommand();
	virtual ~VersionCommand();

	virtual CString GetName();
	virtual CString GetDescription();
	virtual BOOL Execute();
	virtual CString GetErrorString();
	virtual BOOL Match(const CString& strQueryStr);
};

