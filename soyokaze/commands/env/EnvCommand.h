#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace env {


class EnvCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:
	EnvCommand(const CString& name, const CString& value);
	virtual ~EnvCommand();

	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace env
} // end of namespace commands
} // end of namespace soyokaze

