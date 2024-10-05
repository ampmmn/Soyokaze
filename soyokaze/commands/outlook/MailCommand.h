#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace outlook {

class MailItem;

class MailCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	MailCommand(MailItem* itemPtr);
	virtual ~MailCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace outlook
} // end of namespace commands
} // end of namespace launcherapp

