#pragma once

#include "commands/common/UserCommandBase.h"
#include "commands/core/ContextMenuSourceIF.h"
#include <memory>

class CommandFile;

namespace launcherapp {
namespace commands {
namespace shellexecute {

class CommandParam;

class ShellExecCommand :
	virtual public launcherapp::commands::common::UserCommandBase,
	virtual public launcherapp::commands::core::ContextMenuSource
{
public:
	ShellExecCommand();
	virtual ~ShellExecCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	bool CanExecute() override;
	BOOL Execute(Parameter* param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
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

// ContextMenuSource
	// メニューの項目数を取得する
	int GetMenuItemCount() override;
	// メニューの表示名を取得する
	bool GetMenuItemName(int index, LPCWSTR* displayNamePtr) override;
	// メニュー選択時の処理を実行する
	bool SelectMenuItem(int index, launcherapp::core::CommandParameter* param) override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

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
	void SetWorkDir(const CString& path);
	void SetShowType(int showType);
	void SetParam(const CommandParam& param);

	static CString TypeDisplayName();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

