#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class EmptyRecycleBinCommand : public BuiltinCommandBase
{
public:
	EmptyRecycleBinCommand(LPCTSTR name = nullptr);
	virtual ~EmptyRecycleBinCommand();

	HICON GetIcon() override;
	BOOL Execute(const Parameter& param) override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(EmptyRecycleBinCommand)
};

}
}
}
