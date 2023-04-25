#pragma once

#include "CommandIF.h"


class ShellExecCommand : public Command
{
public:
	ShellExecCommand(const CString& name, const CString& path, const CString& param, const CString& dir, const CString& comment, int nShowType);
	virtual ~ShellExecCommand();

	virtual CString GetDescription();

	virtual BOOL Execute();
	virtual CString GetErrorString();
	
	virtual BOOL Match(const CString& strQueryStr);

protected:
	CString m_strName;
	CString m_strPath;
	CString m_strParam;
	CString m_strDir;
	CString m_strComment;
	int m_nShowType;
};
