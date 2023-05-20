#pragma once

#include <vector>
#include "Pattern.h"

class Command
{
public:
	virtual ~Command() {}

	virtual CString GetName() = 0;
	virtual CString GetDescription() = 0;
	virtual BOOL Execute() = 0;
	virtual BOOL Execute(const std::vector<CString>& args) = 0;
	virtual CString GetErrorString() = 0;
	virtual HICON GetIcon() = 0;
	virtual int Match(Pattern* pattern) = 0;
	virtual Command* Clone() = 0;
};

