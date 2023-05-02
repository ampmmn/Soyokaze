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
	virtual BOOL Execute(const std::vector<CString>& args);
	virtual CString GetErrorString();
};

