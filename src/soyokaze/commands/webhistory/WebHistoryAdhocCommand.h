#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/webhistory/WebHistory.h"
#include "commands/core/ExtraCandidateIF.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace webhistory {

class WebHistoryAdhocCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::commands::core::ExtraCandidate
{
public:
	WebHistoryAdhocCommand(const CString& name, const HISTORY& item);
	virtual ~WebHistoryAdhocCommand();

	CString GetName() override;
	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ExtraCandidate
	CString GetSourceName() override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(WebHistoryAdhocCommand)

public:
	static CString TypeDisplayName(LPCTSTR browserName);
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp

