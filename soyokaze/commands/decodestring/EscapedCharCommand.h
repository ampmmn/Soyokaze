#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/common/Clipboard.h"
#include <memory>
#include <string>

namespace launcherapp {
namespace commands {
namespace decodestring {

class EscapedCharCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:

public:
	EscapedCharCommand();
	virtual ~EscapedCharCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeName() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	launcherapp::core::Command* Clone() override;

	static bool ScanAsU4(std::string::iterator& it, std::string::iterator itEnd, std::string& dst);
	static bool ScanAsU8(std::string::iterator& it, std::string::iterator itEnd, std::string& dst);
	static bool ScanAsHex(std::string::iterator& it, std::string::iterator itEnd, std::string& dst);
	static bool ScanAsOctal(std::string::iterator& it, std::string::iterator itEnd, std::string& dst);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace decodestring
} // end of namespace commands
} // end of namespace launcherapp

