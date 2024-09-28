#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/filter/FilterResult.h"
#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace filter {

class CommandParam;

class FilterAdhocCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	FilterAdhocCommand(const CommandParam& param, const FilterResult& result);
	virtual ~FilterAdhocCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeName() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};




} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp


