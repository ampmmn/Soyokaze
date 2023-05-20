#pragma once

#include "CommandIF.h"

class ExecutableFileCommand : public Command
{
public:
	ExecutableFileCommand();
	virtual ~ExecutableFileCommand();

	virtual CString GetName();
	virtual CString GetDescription();
	virtual BOOL Execute();
	virtual BOOL Execute(const std::vector<CString>& args);
	virtual CString GetErrorString();
	virtual HICON GetIcon();
	virtual int Match(Pattern* pattern);
	virtual Command* Clone();

protected:
	struct PImpl;
	PImpl* in;
};
