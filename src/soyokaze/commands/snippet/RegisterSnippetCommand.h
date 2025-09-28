#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace snippet {


class RegisterSnippetCommand : public launcherapp::commands::builtin::BuiltinCommandBase
{
public:
	RegisterSnippetCommand(LPCTSTR name = nullptr);
	virtual ~RegisterSnippetCommand();

	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
	static CString DEFAULT_NAME;

	// BuiltinCommandFactory$B7PM3$G%$%s%9%?%s%9$r@8@.$G$-$k$h$&$K$9$k$?$a$N<jB3$-(B
	DECLARE_BUILTINCOMMAND(RegisterSnippetCommand)
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

