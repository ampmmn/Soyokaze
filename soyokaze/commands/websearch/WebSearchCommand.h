#pragma once

#include "commands/common/UserCommandBase.h"
#include "commands/core/ExtraCandidateSourceIF.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace websearch {

class WebSearchCommand :
 	virtual public launcherapp::commands::common::UserCommandBase,
 	virtual public launcherapp::commands::core::ExtraCandidateSource
{
public:
	WebSearchCommand();
	virtual ~WebSearchCommand();

// Comand
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;
	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsEditable() override;
	int EditDialog(HWND parent) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	bool IsPriorityRankEnabled() override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

// ExtraCandidateSource
	bool QueryCandidates(Pattern* pattern, CommandQueryItemList& commands) override;
	void ClearCache() override;

	static CString GetType();

	static bool NewDialog(Parameter* param, std::unique_ptr<WebSearchCommand>& newCmd);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace websearch
} // end of namespace commands
} // end of namespace launcherapp

