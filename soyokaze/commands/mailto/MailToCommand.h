#pragma once

#include "commands/common/AdhocCommandBase.h"

namespace soyokaze {
namespace commands {
namespace mailto {

class ExecuteHistory;


class MailToCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:
	MailToCommand();
	virtual ~MailToCommand();

	CString GetTypeDisplayName() override;
	BOOL Execute() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	soyokaze::core::Command* Clone() override;
};


} // end of namespace mailto
} // end of namespace commands
} // end of namespace soyokaze

