#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace pathconvert {



class LocalToGitBashPathAdhocCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	LocalToGitBashPathAdhocCommand();
	virtual ~LocalToGitBashPathAdhocCommand();

	CString GetName() override;
	CString GetGuideString() override;
	CString GetTypeName() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	launcherapp::core::Command* Clone() override;

	static bool IsLocalPath(const CString& path);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp


