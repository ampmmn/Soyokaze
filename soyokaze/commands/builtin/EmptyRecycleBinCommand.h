#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace soyokaze {
namespace commands {
namespace builtin {


class EmptyRecycleBinCommand : public BuiltinCommandBase
{
public:
	EmptyRecycleBinCommand(LPCTSTR name = nullptr);
	virtual ~EmptyRecycleBinCommand();

	HICON GetIcon() override;
	BOOL Execute(const Parameter& param) override;
	soyokaze::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
};

}
}
}
