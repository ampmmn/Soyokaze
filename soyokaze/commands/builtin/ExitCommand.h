#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class ExitCommand : public BuiltinCommandBase
{
public:
	ExitCommand(LPCTSTR name = nullptr);
	virtual ~ExitCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
};

}
}
}

