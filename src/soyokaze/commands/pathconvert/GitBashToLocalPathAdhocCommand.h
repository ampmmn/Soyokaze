#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace pathconvert {

class GitBashToLocalPathAdhocCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	GitBashToLocalPathAdhocCommand();
	virtual ~GitBashToLocalPathAdhocCommand();

	CString GetName() override;
	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	launcherapp::core::Command* Clone() override;

	static CString TypeDisplayName();

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(GitBashToLocalPathAdhocCommand)

	static bool IsGitBashPath(const CString& path);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp


