#pragma once

#include "commands/core/CommandIF.h"

namespace launcherapp { namespace commands { namespace error {


// コマンドが実行不可である場合のアクションを実装したコマンド
// このコマンドは特殊でProviderを持たない
class ErrorIndicatorCommand : public launcherapp::core::Command
{
public:
	ErrorIndicatorCommand();
	virtual ~ErrorIndicatorCommand();

	void SetTarget(launcherapp::core::Command* cmd, const String& reason);

	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	CString GetName() override;
	CString GetDescription() override;
	CString GetTypeDisplayName() override;
	bool CanExecute(String*) override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsAllowAutoExecute() override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	Command* Clone() override;
	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;
	uint32_t AddRef() override;
	uint32_t Release() override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



}}} // end of namespace launcherapp::commands::error

