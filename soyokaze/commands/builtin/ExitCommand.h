#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class ExitCommand : public BuiltinCommandBase
{
public:
	ExitCommand(LPCTSTR name = nullptr);
	ExitCommand(const ExitCommand& rhs);
	virtual ~ExitCommand();

	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(ExitCommand)
};

}
}
}

