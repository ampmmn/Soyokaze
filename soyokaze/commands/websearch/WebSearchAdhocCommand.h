#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace websearch {

class WebSearchCommand;

class WebSearchAdhocCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	WebSearchAdhocCommand(WebSearchCommand* baseCommand, const CString& displayName, const CString& showUrl);
	virtual ~WebSearchAdhocCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace websearch
} // end of namespace commands
} // end of namespace launcherapp

