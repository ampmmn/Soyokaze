#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace soyokaze {
namespace commands {
namespace builtin {


class LockScreenCommand : public BuiltinCommandBase
{
public:
	LockScreenCommand(LPCTSTR name = nullptr);
	virtual ~LockScreenCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

