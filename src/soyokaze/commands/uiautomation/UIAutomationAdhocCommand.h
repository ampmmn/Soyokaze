#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ContextMenuSourceIF.h"
#include "commands/core/SelectionBehavior.h"
#include <memory>

namespace launcherapp { namespace commands { namespace uiautomation {

class UIAutomationAdhocCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::commands::core::ContextMenuSource,
	virtual public launcherapp::core::SelectionBehavior
{
public:
	UIAutomationAdhocCommand(HWND hwnd, LPCTSTR name, const CRect& rectWindow, LPCTSTR prefix);
	virtual ~UIAutomationAdhocCommand();

	CString GetName() override;
	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ContextMenuSource
	// メニューの項目数を取得する
	int GetMenuItemCount() override;
	// メニューに対応するアクションを取得する
	bool GetMenuItem(int index, Action** action) override;

// SelectionBehavior
	// 選択された
	void OnSelect(Command* prior) override;
	// 選択解除された
	void OnUnselect(Command* next) override;
	// 実行後のウインドウを閉じる方法を決定する
	CloseWindowPolicy GetCloseWindowPolicy() override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(UIAutomationAdhocCommand)
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}}} // end of namespace launcherapp::commands::uiautomation
