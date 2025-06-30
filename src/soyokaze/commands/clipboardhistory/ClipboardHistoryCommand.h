#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

class ClipboardHistoryCommand :
	virtual	public launcherapp::commands::common::AdhocCommandBase
{
public:
	ClipboardHistoryCommand(const CString& prefix, uint64_t appendData, const CString& data);
	virtual ~ClipboardHistoryCommand();

	CString GetName() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(ClipboardHistoryCommand)

public:
	static CString TypeDisplayName();
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp

