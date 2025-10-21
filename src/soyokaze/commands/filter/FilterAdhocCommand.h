#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ExtraCandidateIF.h"
#include "commands/filter/FilterResult.h"
#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace filter {

class CommandParam;

class FilterAdhocCommand :
 	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::commands::core::ExtraCandidate
{
public:
	FilterAdhocCommand(const CommandParam& param, const FilterResult& result);
	virtual ~FilterAdhocCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ExtraCandidate
	CString GetSourceName() override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(FilterAdhocCommand)

private:
		bool CreatePostFilterSubprocessAction(uint32_t modifierFlags, Action** action);
		bool CreatePostFilterCopyAction(uint32_t modifierFlags, Action** action);
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};




} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp


