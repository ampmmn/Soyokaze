#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class RebootCommand : public BuiltinCommandBase
{
public:
	RebootCommand(LPCTSTR name = nullptr);
	virtual ~RebootCommand();

	HICON GetIcon() override;
	BOOL Execute(const Parameter& param) override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	static launcherapp::core::Command* Create(LPCTSTR);
	DECLARE_BUILTINCOMMAND(RebootCommand)
};

}
}
}
