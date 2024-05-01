#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class ManagerCommand : public BuiltinCommandBase
{
public:
	ManagerCommand(LPCTSTR name = nullptr);
	ManagerCommand(const ManagerCommand& rhs);
	virtual ~ManagerCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;

	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(ManagerCommand)
};


}
}
}
