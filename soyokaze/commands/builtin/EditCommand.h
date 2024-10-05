#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class EditCommand : public BuiltinCommandBase
{
public:
	EditCommand(LPCTSTR name = nullptr);
	EditCommand(const EditCommand& rhs);
	virtual ~EditCommand();

	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(EditCommand)
};

}
}
}
