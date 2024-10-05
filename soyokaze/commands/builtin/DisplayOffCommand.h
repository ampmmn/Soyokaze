#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class DisplayOffCommand : public BuiltinCommandBase
{
public:
	DisplayOffCommand(LPCTSTR name = nullptr);
	DisplayOffCommand(const DisplayOffCommand& rhs);
	virtual ~DisplayOffCommand();

	HICON GetIcon() override;
	BOOL Execute(Parameter* param) override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(DisplayOffCommand)
};

}
}
}
