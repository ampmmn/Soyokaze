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

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
	static CString DEFAULT_NAME;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(RegisterSnippetCommand)
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

