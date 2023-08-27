#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace outlook {

class MailItem;

class MailCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:
	MailCommand(MailItem* itemPtr);
	virtual ~MailCommand();

	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace outlook
} // end of namespace commands
} // end of namespace soyokaze

