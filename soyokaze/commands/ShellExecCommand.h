#pragma once

#include "core/CommandIF.h"
#include <memory>

class HOTKEY_ATTR;


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
	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	soyokaze::core::Command* Clone() override;
	uint32_t AddRef() override;
	uint32_t Release() override;

	// ShellExecCommandのコマンド名として許可しない文字を置換する
	static CString& SanitizeName(CString& str);

	// 管理者権限で実行しているか
	static bool IsRunAsAdmin();
	
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
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
