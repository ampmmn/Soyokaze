#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace presentation {

class PptJumpCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	PptJumpCommand(const CString& filePath, int page, const CString& title);
	virtual ~PptJumpCommand();

	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(PptJumpCommand)

public:
	static CString TypeDisplayName();
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace presentation
} // end of namespace commands
} // end of namespace launcherapp

