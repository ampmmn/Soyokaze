#pragma once

#include "commands/common/AdhocCommandBase.h"

namespace launcherapp {
namespace commands {
namespace mailto {


class MailToCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	MailToCommand();
	virtual ~MailToCommand();

	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	launcherapp::core::Command* Clone() override;

	static CString TypeDisplayName();

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(MailToCommand)
};


} // end of namespace mailto
} // end of namespace commands
} // end of namespace launcherapp

