#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/common/Clipboard.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace timespan {

enum {
	TYPE_HOUR,
	TYPE_MINUTE,
	TYPE_SECOND,
};

class TimespanCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:

public:
	TimespanCommand(CTimeSpan ts, int unitType);
	virtual ~TimespanCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace timespan
} // end of namespace commands
} // end of namespace launcherapp

