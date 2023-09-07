#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace soyokaze {
namespace commands {
namespace builtin {


class DeleteCommand : public BuiltinCommandBase
{
public:
	DeleteCommand(LPCTSTR name = nullptr);
	virtual ~DeleteCommand();

	BOOL Execute(const Parameter& param) override;
	soyokaze::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;
};

}
}
}
