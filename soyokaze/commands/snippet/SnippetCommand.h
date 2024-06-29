#pragma once

#include "commands/common/UserCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace snippet {

class SnippetCommand : public launcherapp::commands::common::UserCommandBase
{
public:
	SnippetCommand();
	virtual ~SnippetCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	int EditDialog(const Parameter* param) override;
	bool IsPriorityRankEnabled() override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

	static CString GetType();

	static bool NewDialog(const Parameter* param);
	
public:
	SnippetCommand& SetName(LPCTSTR name);
	SnippetCommand& SetDescription(LPCTSTR description);
	SnippetCommand& SetText(const CString& text);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

