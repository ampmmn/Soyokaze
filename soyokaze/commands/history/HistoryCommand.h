#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace history {

class HistoryCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:
	HistoryCommand(const CString& keyword);
	virtual ~HistoryCommand();

	CString GetTypeDisplayName() override;
	BOOL Execute() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace history
} // end of namespace commands
} // end of namespace soyokaze

