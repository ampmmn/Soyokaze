#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace soyokaze {
namespace commands {
namespace builtin {


class RebootCommand : public BuiltinCommandBase
{
public:
	RebootCommand(LPCTSTR name = nullptr);
	virtual ~RebootCommand();

	HICON GetIcon() override;
	BOOL Execute(const Parameter& param) override;
	soyokaze::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
};

}
}
}
