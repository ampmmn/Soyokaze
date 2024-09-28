#pragma once

#include "commands/common/UserCommandBase.h"
#include "commands/webhistory/WebHistory.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace webhistory {

class WebHistoryCommand : public launcherapp::commands::common::UserCommandBase
{
public:
	WebHistoryCommand();
	virtual ~WebHistoryCommand();

	bool QueryHistories(Pattern* pattern, std::vector<HISTORY>& histories);
	// 
// Comand
	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeName() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsEditable() override;
	int EditDialog(HWND parent) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	bool IsPriorityRankEnabled() override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

	static CString GetType();

	static bool NewDialog(const Parameter* param, std::unique_ptr<WebHistoryCommand>& newCmd);
	static bool CastFrom(launcherapp::core::Command* cmd, WebHistoryCommand** newCmd); 

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp

