#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/SelectionBehavior.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

class ClipboardHistoryCommand :
	virtual	public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::core::SelectionBehavior
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

// SelectionBehavior
	// $BA*Br$5$l$?(B
	void OnSelect(Command* prior) override;
	// $BA*Br2r=|$5$l$?(B
	void OnUnselect(Command* next) override;
	// $B<B9T8e$N%&%$%s%I%&$rJD$8$kJ}K!$r7hDj$9$k(B
	CloseWindowPolicy GetCloseWindowPolicy() override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

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

