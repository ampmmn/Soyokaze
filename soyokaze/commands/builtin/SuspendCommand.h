#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace soyokaze {
namespace commands {
namespace builtin {


class SuspendCommand : public BuiltinCommandBase
{
public:
	SuspendCommand(LPCTSTR name = nullptr);
	virtual ~SuspendCommand();

	static BOOL DoSuspend(BOOL isSuspend);

	HICON GetIcon() override;
	BOOL Execute(const Parameter& param) override;
	soyokaze::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
};

}
}
}
