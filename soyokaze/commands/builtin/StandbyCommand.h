#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class StandbyCommand : public BuiltinCommandBase
{
public:
	StandbyCommand(LPCTSTR name = nullptr);
	virtual ~StandbyCommand();

	HICON GetIcon() override;
	BOOL Execute(const Parameter& param) override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	static launcherapp::core::Command* Create(LPCTSTR);
	DECLARE_BUILTINCOMMAND(StandbyCommand)
};

}
}
}
