#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {

class RegistWinCommand : public BuiltinCommandBase
{
public:
	RegistWinCommand(LPCTSTR name = nullptr);
	virtual ~RegistWinCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(RegistWinCommand)
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

