#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/uwp/UWPApplicationItem.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace uwp {


class UWPCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:
	UWPCommand(ItemPtr& item);
	virtual ~UWPCommand();

	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace uwp
} // end of namespace commands
} // end of namespace soyokaze

