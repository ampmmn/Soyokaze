#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace soyokaze {
namespace commands {
namespace builtin {

class VersionCommand : public BuiltinCommandBase
{
public:
	VersionCommand(LPCTSTR name = nullptr);
	virtual ~VersionCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

protected:
	bool mIsExecuting;
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

