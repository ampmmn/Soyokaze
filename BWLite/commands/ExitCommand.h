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
	virtual BOOL Execute(const std::vector<CString>& args);
	virtual CString GetErrorString();
	virtual HICON GetIcon();
};
