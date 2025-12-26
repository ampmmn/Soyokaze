#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class NewCommand : public BuiltinCommandBase
{
public:
	NewCommand(LPCTSTR name = nullptr);
	NewCommand(const NewCommand& rhs);
	virtual ~NewCommand();

	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(NewCommand)
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

