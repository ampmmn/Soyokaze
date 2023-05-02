#pragma once

#include "CommandIF.h"

class ShellExecCommand : public Command
{
public:
	struct ATTRIBUTE {
		ATTRIBUTE();

		CString mPath;
		CString mParam;
		CString mDir;
		int mShowType;
	};


public:
	ShellExecCommand();
	virtual ~ShellExecCommand();

	virtual CString GetName();
	virtual CString GetDescription();

	virtual BOOL Execute();
	virtual BOOL Execute(const std::vector<CString>& args);
	virtual CString GetErrorString();
	
public:
	ShellExecCommand& SetName(LPCTSTR name);
	ShellExecCommand& SetDescription(LPCTSTR description);
	ShellExecCommand& SetAttribute(const ATTRIBUTE& attr);
	ShellExecCommand& SetAttributeForParam0(const ATTRIBUTE& attr);
	ShellExecCommand& SetPath(LPCTSTR path);
	ShellExecCommand& SetRunAs(int runAs);

protected:
	ATTRIBUTE& SelectAttribute(const std::vector<CString>& args);

public:
	static void ExpandArguments(const std::vector<CString>& args, CString& path, CString& param);
	static void ExpandEnv(CString& texts);

protected:
	CString mName;
	CString mDescription;
	int mRunAs;

	ATTRIBUTE mNormalAttr;
	ATTRIBUTE mNoParamAttr;
};
