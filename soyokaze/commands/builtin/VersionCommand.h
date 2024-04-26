#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {

class VersionCommand : public BuiltinCommandBase
{
public:
	VersionCommand(LPCTSTR name = nullptr);
	virtual ~VersionCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	static launcherapp::core::Command* Create(LPCTSTR);
	DECLARE_BUILTINCOMMAND(VersionCommand)

protected:
	bool mIsExecuting;
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

