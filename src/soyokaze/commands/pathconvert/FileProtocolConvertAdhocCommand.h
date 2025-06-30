#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace pathconvert {



class FileProtocolConvertAdhocCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	FileProtocolConvertAdhocCommand();
	virtual ~FileProtocolConvertAdhocCommand();

	CString GetName() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	launcherapp::core::Command* Clone() override;

	static CString TypeDisplayName();

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(FileProtocolConvertAdhocCommand)

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp


