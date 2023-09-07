#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace soyokaze {
namespace commands {
namespace builtin {


class NewCommand : public BuiltinCommandBase
{
public:
	NewCommand(LPCTSTR name = nullptr);
	virtual ~NewCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

