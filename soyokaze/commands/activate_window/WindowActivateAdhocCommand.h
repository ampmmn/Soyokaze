#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ContextMenuSourceIF.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace activate_window {

class MenuEventListener
{
public:
	virtual ~MenuEventListener() {}
	virtual void OnRequestPutName(HWND hwnd) = 0;
	virtual void OnRequestClose(HWND hwnd) = 0;
};

class WindowActivateAdhocCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::commands::core::ContextMenuSource

{
public:
	WindowActivateAdhocCommand(HWND hwnd);
	virtual ~WindowActivateAdhocCommand();

	void SetListener(MenuEventListener* listener);

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ContextMenuSource
	// メニューの項目数を取得する
	int GetMenuItemCount() override;
	// メニューの表示名を取得する
	bool GetMenuItemName(int index, LPCWSTR* displayNamePtr) override;
	// メニュー選択時の処理を実行する
	bool SelectMenuItem(int index, launcherapp::core::CommandParameter* param) override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(WindowActivateAdhocCommand)
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

