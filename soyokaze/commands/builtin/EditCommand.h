#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class EditCommand : public BuiltinCommandBase
{
public:
	EditCommand(LPCTSTR name = nullptr);
	virtual ~EditCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
};

}
}
}
