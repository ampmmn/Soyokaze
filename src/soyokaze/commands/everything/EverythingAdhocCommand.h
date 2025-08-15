#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ExtraCandidateIF.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace everything {

class CommandParam;
class EverythingResult;

class EverythingAdhocCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::commands::core::ExtraCandidate
{
public:
	EverythingAdhocCommand();
	EverythingAdhocCommand(const CommandParam& param, const EverythingResult& result);
	virtual ~EverythingAdhocCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ExtraCandidate
	CString GetSourceName() override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(EverythingAdhocCommand)

public:
	static CString TypeDisplayName();
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

