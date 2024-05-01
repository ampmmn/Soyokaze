#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class SuspendCommand : public BuiltinCommandBase
{
public:
	SuspendCommand(LPCTSTR name = nullptr);
	SuspendCommand(const SuspendCommand& rhs);
	virtual ~SuspendCommand();

	static BOOL DoSuspend(BOOL isSuspend);

	HICON GetIcon() override;
	BOOL Execute(const Parameter& param) override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(SuspendCommand)
};

}
}
}
