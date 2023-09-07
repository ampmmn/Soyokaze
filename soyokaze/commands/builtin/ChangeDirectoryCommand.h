#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace soyokaze {
namespace commands {
namespace builtin {


class ChangeDirectoryCommand : public BuiltinCommandBase
{
public:
	ChangeDirectoryCommand(LPCTSTR name = nullptr);
	virtual ~ChangeDirectoryCommand();

	BOOL Execute(const Parameter& param) override;
	soyokaze::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
};

}
}
}
