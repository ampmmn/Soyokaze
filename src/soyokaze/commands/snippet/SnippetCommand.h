#pragma once

#include "commands/common/UserCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace snippet {

class CommandParam;

class SnippetCommand :
 virtual	public launcherapp::commands::common::UserCommandBase
{
public:
	SnippetCommand();
	virtual ~SnippetCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetTypeDisplayName() override;

	bool GetAction(uint32_t modifierFlags, Action** action) override;
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

	static CString GetType();
	static CString TypeDisplayName();

	static bool NewDialog(Parameter* param);
	
public:
	void SetParam(const CommandParam& param);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

