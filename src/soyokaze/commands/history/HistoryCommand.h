#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace history {

class HistoryCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	HistoryCommand(const CString& keyword);
	virtual ~HistoryCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(HistoryCommand)
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace history
} // end of namespace commands
} // end of namespace launcherapp

