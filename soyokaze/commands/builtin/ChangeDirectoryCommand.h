#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class ChangeDirectoryCommand : public BuiltinCommandBase
{
public:
	ChangeDirectoryCommand(LPCTSTR name = nullptr);
	virtual ~ChangeDirectoryCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(ChangeDirectoryCommand)
};

}
}
}
