#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class RebootCommand : public BuiltinCommandBase
{
public:
	RebootCommand(LPCTSTR name = nullptr);
	RebootCommand(const RebootCommand& rhs);
	virtual ~RebootCommand();

	HICON GetIcon() override;
	BOOL Execute(const Parameter& param) override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(RebootCommand)
};

}
}
}
