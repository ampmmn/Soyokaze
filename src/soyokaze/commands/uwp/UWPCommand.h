#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/uwp/UWPApplicationItem.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace uwp {


class UWPCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	UWPCommand(ItemPtr& item);
	virtual ~UWPCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(UWPCommand)

public:
	static CString TypeDisplayName();
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace uwp
} // end of namespace commands
} // end of namespace launcherapp

