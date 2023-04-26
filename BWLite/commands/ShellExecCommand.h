#pragma once

#include "CommandIF.h"


class ShellExecCommand : public Command
{
public:
	ShellExecCommand();
	virtual ~ShellExecCommand();

	virtual CString GetName();
	virtual CString GetDescription();

	virtual BOOL Execute();
	virtual CString GetErrorString();
	
	virtual BOOL Match(const CString& strQueryStr);

public:
	ShellExecCommand& SetName(LPCTSTR name);
	ShellExecCommand& SetPath(LPCTSTR path);
	ShellExecCommand& SetParam(LPCTSTR param);
	ShellExecCommand& SetDirectory(LPCTSTR dir);
	ShellExecCommand& SetDescription(LPCTSTR description);
	ShellExecCommand& SetShowType(int showType);
	ShellExecCommand& SetEnable(bool isEnable);

protected:
	CString m_strName;
	CString m_strNameForIgnoreCase;
	CString m_strPath;
	CString m_strParam;
	CString m_strDir;
	CString m_strDescription;
	int m_nShowType;
	bool m_bEnable;
};
