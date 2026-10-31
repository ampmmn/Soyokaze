#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/PreviewProvider.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

class ClipboardHistoryCommand :
	virtual	public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::core::PreviewProvider
{
public:
	ClipboardHistoryCommand(const CString& prefix, uint64_t appendData, const CString& data);
	virtual ~ClipboardHistoryCommand();

	CString GetName() override;
	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// PreviewProvider
	launcherapp::mainwindow::preview::PreviewWindow* FetchPreviewWindow() override;

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

