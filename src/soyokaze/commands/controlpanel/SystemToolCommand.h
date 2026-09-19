#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ContextMenuSourceIF.h"
#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace controlpanel {

class SystemToolCommand :
	public launcherapp::commands::common::AdhocCommandBase,
	public launcherapp::commands::core::ContextMenuSource
{
public:
	SystemToolCommand(const CString& name, const CString& description, const std::vector<CString>& commandLine);
	virtual ~SystemToolCommand();

	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	// メニューの項目数を取得する
	int GetMenuItemCount() override;
	// メニューに対応するアクションを取得する
	bool GetMenuItem(int index, Action** action) override;

	// インターフェースを取得する
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(SystemToolCommand)

public:
	static CString TypeDisplayName();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // controlpanel名前空間終了
} // commands名前空間終了
} // launcherapp名前空間終了
