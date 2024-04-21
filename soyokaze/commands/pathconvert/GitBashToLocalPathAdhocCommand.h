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
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	launcherapp::core::Command* Clone() override;

	static bool IsGitBashPath(const CString& path);

protected:
	static bool ShouldCopy(const Parameter& param);

	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp


