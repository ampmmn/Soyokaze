#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace controlpanel {

class ControlPanelCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:
	ControlPanelCommand(const CString& name, const CString& iconPath, const CString& command, const CString& description);
	virtual ~ControlPanelCommand();

	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace controlpanel
} // end of namespace commands
} // end of namespace soyokaze

