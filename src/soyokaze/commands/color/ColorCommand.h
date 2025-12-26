#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/common/Clipboard.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace color {

enum {
	TYPE_HEX6,
	TYPE_HEX3,
	TYPE_RGB,
	TYPE_HSL,
};

class ColorCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:

public:
	ColorCommand(COLORREF cr, int formatType);
	virtual ~ColorCommand();

	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(ColorCommand)

public:
	static CString TypeDisplayName();
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace color
} // end of namespace commands
} // end of namespace launcherapp

