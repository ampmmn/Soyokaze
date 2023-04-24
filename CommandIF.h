#pragma once


class Command
{
public:
	virtual ~Command() {}

	virtual CString GetDescription() = 0;
	virtual BOOL Execute() = 0;
	virtual CString GetErrorString() = 0;
	virtual BOOL Match(const CString& strQueryStr) = 0;
};

