#pragma once

class Command
{
public:
	virtual ~Command() {}

	virtual CString GetName() = 0;
	virtual CString GetDescription() = 0;
	virtual BOOL Execute() = 0;
	virtual CString GetErrorString() = 0;
};

