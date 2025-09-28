#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace activate_worksheet {

class Worksheet;

class WorksheetCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	WorksheetCommand(Worksheet* sheet);
	virtual ~WorksheetCommand();

	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(WorksheetCommand)

public:
	static CString TypeDisplayName();
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace activate_worksheet
} // end of namespace commands
} // end of namespace launcherapp

