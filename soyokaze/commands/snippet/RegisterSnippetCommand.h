#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace soyokaze {
namespace commands {
namespace snippet {


class RegisterSnippetCommand : public soyokaze::commands::builtin::BuiltinCommandBase
{
public:
	RegisterSnippetCommand(LPCTSTR name = nullptr);
	virtual ~RegisterSnippetCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
	static CString DEFAULT_NAME;
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

