#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace soyokaze {
namespace commands {
namespace builtin {


class StandbyCommand : public BuiltinCommandBase
{
public:
	StandbyCommand(LPCTSTR name = nullptr);
	virtual ~StandbyCommand();

	HICON GetIcon() override;
	BOOL Execute(const Parameter& param) override;
	soyokaze::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
};

}
}
}
