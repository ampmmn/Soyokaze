#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class LogOffCommand : public BuiltinCommandBase
{
public:
	LogOffCommand(LPCTSTR name = nullptr);
	virtual ~LogOffCommand();

	HICON GetIcon() override;
	BOOL Execute(const Parameter& param) override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
};

}
}
}
