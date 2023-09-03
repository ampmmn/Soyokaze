#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/common/Clipboard.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace decodestring {

class EscapedCharCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:

public:
	EscapedCharCommand();
	virtual ~EscapedCharCommand();

	CString GetName() override;
	CString GetDescription() override;

	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	soyokaze::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace decodestring
} // end of namespace commands
} // end of namespace soyokaze

