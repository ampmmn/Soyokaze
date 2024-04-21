#pragma once

#include "commands/common/AdhocCommandBase.h"

namespace launcherapp {
namespace commands {
namespace mailto {

class ExecuteHistory;


class MailToCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	MailToCommand();
	virtual ~MailToCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	launcherapp::core::Command* Clone() override;
};


} // end of namespace mailto
} // end of namespace commands
} // end of namespace launcherapp

