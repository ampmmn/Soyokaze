#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace soyokaze {
namespace commands {
namespace builtin {


class ManagerCommand : public BuiltinCommandBase
{
public:
	ManagerCommand(LPCTSTR name = nullptr);
	virtual ~ManagerCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;

	soyokaze::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
};


}
}
}
