#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace pathfind {

class ExcludePathList;


class PathExecuteCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	PathExecuteCommand(ExcludePathList* excludeList = nullptr);
	virtual ~PathExecuteCommand();

	void SetFullPath(const CString& path, bool isFromHistory);
	void Reload();

	CString GetName() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	launcherapp::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace launcherapp

