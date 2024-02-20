#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace soyokaze {
namespace commands {
namespace builtin {


class AfxChangeDirectoryCommand : public BuiltinCommandBase
{
public:
	AfxChangeDirectoryCommand(LPCTSTR name = nullptr);
	virtual ~AfxChangeDirectoryCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
};
	
}
}
}
