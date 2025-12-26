#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ContextMenuSourceIF.h"
#include "commands/specialfolderfiles/SpecialFolderFile.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace specialfolderfiles {


class SpecialFolderFileCommand : public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::commands::core::ContextMenuSource

{
public:
	SpecialFolderFileCommand(const ITEM& item);
	virtual ~SpecialFolderFileCommand();

	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ContextMenuSource
	// メニューの項目数を取得する
	int GetMenuItemCount() override;
	// メニューに対応するアクションを取得する
	bool GetMenuItem(int index, Action** action) override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

public:
	static CString TypeDisplayName(int index);

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(SpecialFolderFileCommand)
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace launcherapp

