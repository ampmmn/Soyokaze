#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class ReloadCommand : public BuiltinCommandBase
{
public:
	ReloadCommand(LPCTSTR name = nullptr);
	ReloadCommand(const ReloadCommand& rhs);
	virtual ~ReloadCommand();

	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	LRESULT OnCallbackExecute();

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(ReloadCommand)
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

