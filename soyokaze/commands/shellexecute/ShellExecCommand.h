#pragma once

#include "commands/common/UserCommandBase.h"
#include <memory>

class CommandFile;

namespace launcherapp {
namespace commands {
namespace shellexecute {

class ShellExecCommand : public launcherapp::commands::common::UserCommandBase
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
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	int EditDialog(HWND parent) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	bool IsPriorityRankEnabled() override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

	static CString GetType();

	static bool NewDialog(const Parameter* param, ShellExecCommand** newCmd);
	static bool LoadFrom(CommandFile* cmdFile, void* entry,ShellExecCommand** newCmdPtr);

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

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

