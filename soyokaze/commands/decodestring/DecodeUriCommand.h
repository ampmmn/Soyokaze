#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/common/Clipboard.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace decodestring {

class DecodeUriCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:

public:
	DecodeUriCommand();
	virtual ~DecodeUriCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeName() override;

	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	launcherapp::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace decodestring
} // end of namespace commands
} // end of namespace launcherapp

