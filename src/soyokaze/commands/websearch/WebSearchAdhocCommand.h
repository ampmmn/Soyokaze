#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ContextMenuSourceIF.h"
#include "commands/core/ExtraCandidateIF.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace websearch {

class WebSearchCommand;

class WebSearchAdhocCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::commands::core::ContextMenuSource,
	virtual public launcherapp::commands::core::ExtraCandidate
{
public:
	WebSearchAdhocCommand(WebSearchCommand* baseCommand, const CString& displayName, const CString& showUrl);
	virtual ~WebSearchAdhocCommand();

	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ContextMenuSource
	// メニューの項目数を取得する
	int GetMenuItemCount() override;
	// メニューに対応するアクションを取得する
	bool GetMenuItem(int index, Action** action) override;

// ExtraCandidate
	CString GetSourceName() override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(WebSearchAdhocCommand)
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace websearch
} // end of namespace commands
} // end of namespace launcherapp

