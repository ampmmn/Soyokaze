#pragma once

#include "CommandIF.h"

class MainDirCommand : public Command
{
public:
	MainDirCommand();
	virtual ~MainDirCommand();

	virtual CString GetName();
	virtual CString GetDescription();
	virtual BOOL Execute();
	virtual BOOL Execute(const std::vector<CString>& args);
	virtual CString GetErrorString();
	virtual HICON GetIcon();
	virtual int Match(Pattern* pattern);
	virtual Command* Clone();
};

