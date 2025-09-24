#pragma once

#include "commands/common/UserCommandBase.h"
#include <memory>

class CommandFile;

namespace launcherapp {
namespace commands {
namespace align_window {

class CommandParam;

class AlignWindowCommand : public launcherapp::commands::common::UserCommandBase
{
public:
	AlignWindowCommand();
	virtual ~AlignWindowCommand();

	void SetParam(const CommandParam& param);

// Command
	CString GetName() override;
	CString GetDescription() override;
	CString GetTypeDisplayName() override;

	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsAllowAutoExecute() override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

// Editable
	// コマンドを編集するためのダイアログを作成/取得する
	bool CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor) override;
	// ダイアログ上での編集結果をコマンドに適用する
	bool Apply(launcherapp::core::CommandEditor* editor) override;
	// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
	bool CreateNewInstanceFrom(launcherapp::core::CommandEditor*editor, Command** newCmd) override;

	static CString GetType();
	static CString TypeDisplayName();

	static bool NewDialog(Parameter* param, AlignWindowCommand** newCmd);
	static bool LoadFrom(CommandFile* cmdFile, void* entry, AlignWindowCommand** newCmdPtr);

private:
	// 対象のウインドウを整列
	bool AlignTarget(HWND& prevForegroundHwnd, String* errMsg);
	// 対象を前面にセット
	bool SetForeground(HWND prevForegroundHwnd, String* errMsg);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace align_window
} // end of namespace commands
} // end of namespace launcherapp

