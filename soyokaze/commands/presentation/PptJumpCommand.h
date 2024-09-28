#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace presentation {

class PptJumpCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	PptJumpCommand(int page, const CString& title);
	virtual ~PptJumpCommand();

	CString GetGuideString() override;
	CString GetTypeName() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace presentation
} // end of namespace commands
} // end of namespace launcherapp

