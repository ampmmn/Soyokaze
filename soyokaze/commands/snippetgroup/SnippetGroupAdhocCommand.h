#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace snippetgroup {

class SnippetGroupParam;
class Item;

class SnippetGroupAdhocCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	SnippetGroupAdhocCommand(const SnippetGroupParam& param, const Item& item);
	virtual ~SnippetGroupAdhocCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace snippetgroup
} // end of namespace commands
} // end of namespace launcherapp

