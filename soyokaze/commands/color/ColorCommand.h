#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace color {

class ColorCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:
	ColorCommand(COLORREF cr);
	virtual ~ColorCommand();

	CString GetTypeDisplayName() override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace color
} // end of namespace commands
} // end of namespace soyokaze

