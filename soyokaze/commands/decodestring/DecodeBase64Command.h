#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/common/Clipboard.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace decodestring {

class DecodeBase64Command : public launcherapp::commands::common::AdhocCommandBase
{
public:

public:
	DecodeBase64Command();
	virtual ~DecodeBase64Command();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeName() override;

	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	launcherapp::core::Command* Clone() override;
};


} // end of namespace decodestring
} // end of namespace commands
} // end of namespace launcherapp

