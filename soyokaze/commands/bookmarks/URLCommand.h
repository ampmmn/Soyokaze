#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ExtraCandidateIF.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace bookmarks {

class URLCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::commands::core::ExtraCandidate
{
public:
public:
	URLCommand(const CString& browserName, const CString& name, const CString& url);
	virtual ~URLCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ExtraCandidate
	CString GetSourceName() override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(URLCommand)
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

