#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace activate_worksheet {

class CalcWorksheet;

class CalcWorksheetCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	CalcWorksheetCommand(CalcWorksheet* sheet);
	virtual ~CalcWorksheetCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(CalcWorksheetCommand)

public:
	static CString TypeDisplayName();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace activate_worksheet
} // end of namespace commands
} // end of namespace launcherapp

