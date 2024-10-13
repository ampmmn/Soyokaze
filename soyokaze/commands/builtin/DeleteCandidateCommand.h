#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace builtin {

class WebSearchCommand;

class DeleteCandidateCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	DeleteCandidateCommand(const CString& cmdName);
	virtual ~DeleteCandidateCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

