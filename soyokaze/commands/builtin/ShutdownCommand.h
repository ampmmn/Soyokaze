#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace soyokaze {
namespace commands {
namespace builtin {


class ShutdownCommand : public BuiltinCommandBase
{
public:
	ShutdownCommand(LPCTSTR name = nullptr);
	virtual ~ShutdownCommand();

	static BOOL DoExit(UINT uFlags);

	HICON GetIcon() override;
	BOOL Execute(const Parameter& param) override;
	soyokaze::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
};

}
}
}
