#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/webhistory/WebHistory.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace webhistory {

class WebHistoryAdhocCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	WebHistoryAdhocCommand(const HISTORY& item);
	virtual ~WebHistoryAdhocCommand();

	CString GetGuideString() override;
	CString GetTypeName() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp

