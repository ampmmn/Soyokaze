#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommand.h"
#include "commands/url_directoryindex/DirectoryIndexQueryResult.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace url_directoryindex {

class CommandParam;

class DirectoryIndexAdhocCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	DirectoryIndexAdhocCommand(URLDirectoryIndexCommand* baseCmd, const QueryResult& result);
	virtual ~DirectoryIndexAdhocCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeName() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};




} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp


