#pragma once

#include "CommandIF.h"

class UserDirCommand : public Command
{
public:
	UserDirCommand();
	virtual ~UserDirCommand();

	virtual CString GetName();
	virtual CString GetDescription();
	virtual BOOL Execute();
	virtual BOOL Execute(const std::vector<CString>& args);
	virtual CString GetErrorString();
	virtual HICON GetIcon();
	virtual BOOL Match(Pattern* pattern);
};



