#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/common/Clipboard.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace color {

enum {
	TYPE_HEX6,
	TYPE_HEX3,
	TYPE_RGB,
	TYPE_HSL,
};

class ColorCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:

public:
	ColorCommand(COLORREF cr, int formatType);
	virtual ~ColorCommand();

	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace color
} // end of namespace commands
} // end of namespace soyokaze

