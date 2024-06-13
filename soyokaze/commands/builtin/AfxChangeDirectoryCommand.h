#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class AfxChangeDirectoryCommand : public BuiltinCommandBase
{
public:
	AfxChangeDirectoryCommand(LPCTSTR name = nullptr);
	AfxChangeDirectoryCommand(const AfxChangeDirectoryCommand& rhs);
	virtual ~AfxChangeDirectoryCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	DECLARE_BUILTINCOMMAND(AfxChangeDirectoryCommand)
};
	
}
}
}
