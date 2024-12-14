#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/mmc/MMCSnapin.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace mmc {

class MMCCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	MMCCommand(const MMCSnapin& snapin);
	virtual ~MMCCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(MMCCommand)
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace mmc
} // end of namespace commands
} // end of namespace launcherapp

