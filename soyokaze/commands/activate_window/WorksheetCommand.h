#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace activate_window {

class Worksheet;

class WorksheetCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	WorksheetCommand(Worksheet* sheet);
	virtual ~WorksheetCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

