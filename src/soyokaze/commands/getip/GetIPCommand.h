#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/common/Clipboard.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace getip {

class GetIPCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:

public:
	GetIPCommand(const CString& displayName, const CString& addr);
	virtual ~GetIPCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(GetIPCommand)

public:
	static CString TypeDisplayName();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace getip
} // end of namespace commands
} // end of namespace launcherapp

