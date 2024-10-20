#pragma once

#include "commands/common/UserCommandBase.h"
#include <memory>

class CommandFile;

namespace launcherapp {
namespace commands {
namespace shellexecute {

class CommandParam;

class ShellExecCommand :
	virtual public launcherapp::commands::common::UserCommandBase
{
public:
	ShellExecCommand();
	virtual ~ShellExecCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(Parameter* param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	bool IsPriorityRankEnabled() override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

// Editable
	// コマンドを編集するためのダイアログを作成/取得する
	virtual bool CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor) override;
	// ダイアログ上での編集結果をコマンドに適用する
	virtual bool Apply(launcherapp::core::CommandEditor* editor) override;
	// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
	virtual bool CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmd) override;

	static CString GetType();

	static bool NewDialog(Parameter* param, ShellExecCommand** newCmd);
	static bool NewCommand(const CString& filePath);
	static bool LoadFrom(CommandFile* cmdFile, void* entry,ShellExecCommand** newCmdPtr);

private:
	// ShellExecCommandのコマンド名として許可しない文字を置換する
	static CString& SanitizeName(CString& str);
	
public:
	void SetPath(const CString& path);
	void SetArgument(const CString& arg);
	void SetParam(const CommandParam& param);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

