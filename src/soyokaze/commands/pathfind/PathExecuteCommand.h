#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ContextMenuSourceIF.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace pathfind {

class ExcludePathList;


class PathExecuteCommand :
 	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::commands::core::ContextMenuSource
{
public:
	PathExecuteCommand(ExcludePathList* excludeList = nullptr);
	virtual ~PathExecuteCommand();

	void Reload();

	CString GetName() override;
	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	launcherapp::core::Command* Clone() override;

// ContextMenuSource
	// メニューの項目数を取得する
	int GetMenuItemCount() override;
	// メニューの表示名を取得する
	bool GetMenuItemName(int index, LPCWSTR* displayNamePtr) override;
	// メニュー選択時の処理を実行する
	bool SelectMenuItem(int index, Parameter* param) override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	static CString TypeDisplayName();

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(PathExecuteCommand)

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace launcherapp

