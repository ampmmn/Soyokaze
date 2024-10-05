#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace env {


class EnvCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	EnvCommand(const CString& name, const CString& value);
	virtual ~EnvCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace env
} // end of namespace commands
} // end of namespace launcherapp

