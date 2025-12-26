#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace unitconvert {

class InchAdhocCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	InchAdhocCommand();
	virtual ~InchAdhocCommand();

	CString GetName() override;
	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(InchAdhocCommand)

public:
	static CString TypeDisplayName();
protected:

	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace unitconvert
} // end of namespace commands
} // end of namespace launcherapp


