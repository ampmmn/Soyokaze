#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ExtraCandidateIF.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace snippetgroup {

class SnippetGroupParam;
class Item;

class SnippetGroupAdhocCommand :
	virtual	public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::commands::core::ExtraCandidate
{
public:
	SnippetGroupAdhocCommand(const SnippetGroupParam& param, const Item& item);
	virtual ~SnippetGroupAdhocCommand();

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

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(SnippetGroupAdhocCommand)
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace snippetgroup
} // end of namespace commands
} // end of namespace launcherapp

