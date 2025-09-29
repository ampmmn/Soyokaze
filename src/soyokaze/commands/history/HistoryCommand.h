#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ContextMenuSourceIF.h"
#include <memory>

namespace launcherapp { namespace commands { namespace history {

class HistoryCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::commands::core::ContextMenuSource
{
public:
	HistoryCommand(const CString& keyword);
	virtual ~HistoryCommand();

	CString GetDescription() override;
	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ContextMenuSource
	// メニューの項目数を取得する
	int GetMenuItemCount() override;
	// メニューに対応するアクションを取得する
	bool GetMenuItem(int index, Action** action) override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(HistoryCommand)

public:
	static CString TypeDisplayName();
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::history

