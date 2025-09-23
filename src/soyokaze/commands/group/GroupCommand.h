#pragma once

#include "commands/common/UserCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace group {

class CommandParam;

// グループコマンド
class GroupCommand : virtual public launcherapp::commands::common::UserCommandBase
{
	class Exception;
public:
	GroupCommand();
	virtual ~GroupCommand();

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
	virtual bool CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor) override;
	// ダイアログ上での編集結果をコマンドに適用する
	virtual bool Apply(launcherapp::core::CommandEditor* editor) override;
	// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
	virtual bool CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmd) override;

	static CString GroupCommand::GetType();
	static CString TypeDisplayName();

	static bool NewDialog(Parameter* param);
public:
	void SetParam(const CommandParam& param);

	void AddItem(LPCTSTR itemName, bool isWait);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp

