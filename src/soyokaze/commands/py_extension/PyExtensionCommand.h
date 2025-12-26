#pragma once

#include "commands/common/UserCommandBase.h"
#include <memory>

class CommandFile;

namespace launcherapp { namespace commands { namespace py_extension {

class CommandParam;

class PyExtensionCommand :
 virtual public launcherapp::commands::common::UserCommandBase
{
public:
	PyExtensionCommand();
	virtual ~PyExtensionCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetTypeDisplayName() override;

	// 修飾キー押下状態に対応した実行アクションを取得する
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsAllowAutoExecute() override;
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

	static CString GetType();
	static CString TypeDisplayName();

	static bool NewInstance(launcherapp::core::CommandEditor* editor, Command** newCmdPtr);
	static bool NewDialog(Parameter* param, PyExtensionCommand** newCmdPtrd);
	static bool LoadFrom(CommandFile* cmdFile, void* entry, PyExtensionCommand** newCmdPtr);
	
public:
	void SetParam(const CommandParam& param);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}}}

