#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class RestartCommand : public BuiltinCommandBase
{
public:
	RestartCommand(LPCTSTR name = nullptr);
	RestartCommand(const RestartCommand& rhs);
	virtual ~RestartCommand();

	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(RestartCommand)
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

