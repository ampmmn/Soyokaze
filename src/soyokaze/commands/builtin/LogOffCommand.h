#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class LogOffCommand : public BuiltinCommandBase
{
public:
	LogOffCommand(LPCTSTR name = nullptr);
	LogOffCommand(const LogOffCommand& rhs);
	virtual ~LogOffCommand();

	HICON GetIcon() override;
	BOOL Execute(Parameter* param) override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(LogOffCommand)
};

}
}
}
