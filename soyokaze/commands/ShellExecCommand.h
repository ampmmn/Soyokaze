#pragma once

#include "core/CommandIF.h"

class ShellExecCommand : public soyokaze::core::Command
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

	CString GetName() override;
	CString GetDescription() override;

	BOOL Execute() override;
	BOOL Execute(const std::vector<CString>& args) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	soyokaze::core::Command* Clone() override;
	uint32_t AddRef() override;
	uint32_t Release() override;

	// ShellExecCommand$B$N%3%^%s%IL>$H$7$F5v2D$7$J$$J8;z$rCV49$9$k(B
	static CString& SanitizeName(CString& str);
	
public:
	ShellExecCommand& SetName(LPCTSTR name);
	ShellExecCommand& SetDescription(LPCTSTR description);
	ShellExecCommand& SetAttribute(const ATTRIBUTE& attr);
	ShellExecCommand& SetAttributeForParam0(const ATTRIBUTE& attr);
	ShellExecCommand& SetPath(LPCTSTR path);
	ShellExecCommand& SetRunAs(int runAs);

	void GetAttribute(ATTRIBUTE& attr);
	void GetAttributeForParam0(ATTRIBUTE& attr);
	int GetRunAs();

protected:
	void SelectAttribute(const std::vector<CString>& args,ATTRIBUTE& attr);

public:
	static void ExpandArguments(const std::vector<CString>& args, CString& path, CString& param);
	static void ExpandEnv(CString& texts);

protected:
	CString mName;
	CString mDescription;
	int mRunAs;

	ATTRIBUTE mNormalAttr;
	ATTRIBUTE mNoParamAttr;

	CString mErrMsg;

	uint32_t mRefCount;
};
