#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/filter/FilterCommandParam.h"
#include "commands/filter/FilterResult.h"

namespace launcherapp { namespace commands { namespace filter {

class PostFilterCommand : virtual public launcherapp::commands::common::AdhocCommandBase
{
public:
	PostFilterCommand(const CommandParam& param, const FilterResult& result);
	virtual ~PostFilterCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(PostFilterCommand)

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

}}}


