#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace soyokaze {
namespace commands {
namespace builtin {


class MainDirCommand : public BuiltinCommandBase
{
public:
	MainDirCommand(LPCTSTR name = nullptr);
	virtual ~MainDirCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
};


}
}
}
