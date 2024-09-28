#pragma once

#include "commands/common/UserCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace group {

class CommandParam;

// グループコマンド
class GroupCommand : public launcherapp::commands::common::UserCommandBase
{
	class Exception;
public:
	GroupCommand();
	virtual ~GroupCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeName() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	int EditDialog(HWND parent) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	bool IsPriorityRankEnabled() override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

	static CString GroupCommand::GetType();

public:
	void SetParam(const CommandParam& param);

	void AddItem(LPCTSTR itemName, bool isWait);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp

