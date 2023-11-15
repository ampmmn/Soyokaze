#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/common/Clipboard.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace timespan {

enum {
	TYPE_HOUR,
	TYPE_MINUTE,
	TYPE_SECOND,
};

class TimespanCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:

public:
	TimespanCommand(CTimeSpan ts, int unitType);
	virtual ~TimespanCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace timespan
} // end of namespace commands
} // end of namespace soyokaze

