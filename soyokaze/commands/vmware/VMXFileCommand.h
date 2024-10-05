#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace vmware {


class VMXFileCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	VMXFileCommand(const CString& name, const CString& fullPath);
	virtual ~VMXFileCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	bool IsPriorityRankEnabled() override;
	launcherapp::core::Command* Clone() override;


protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace vmware
} // end of namespace commands
} // end of namespace launcherapp

