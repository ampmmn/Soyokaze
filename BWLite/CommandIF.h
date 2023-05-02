#pragma once

#include <vector>

class Command
{
public:
	virtual ~Command() {}

	virtual CString GetName() = 0;
	virtual CString GetDescription() = 0;
	virtual BOOL Execute() = 0;
	virtual BOOL Execute(const std::vector<CString>& args) = 0;
	virtual CString GetErrorString() = 0;
};

