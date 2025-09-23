#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/winscp/WinScpCommandParam.h"
#include <memory>

namespace launcherapp { namespace commands { namespace winscp {


class WinScpCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase
{
public:
	WinScpCommand(CommandParam* param, const CString& sessionName);
	virtual ~WinScpCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(WinScpCommand)

public:
	static CString TypeDisplayName();
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::winscp

