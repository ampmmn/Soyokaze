#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {

class UserDirCommand : public BuiltinCommandBase
{
public:
	UserDirCommand(LPCTSTR name = nullptr);
	UserDirCommand(const UserDirCommand& rhs);
	virtual ~UserDirCommand();

	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(UserDirCommand)
};



} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

