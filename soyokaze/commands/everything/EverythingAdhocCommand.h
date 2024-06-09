#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace everything {

class CommandParam;
class EverythingResult;

class EverythingAdhocCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	EverythingAdhocCommand(const CommandParam& param, const EverythingResult& result);
	virtual ~EverythingAdhocCommand();

		CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

