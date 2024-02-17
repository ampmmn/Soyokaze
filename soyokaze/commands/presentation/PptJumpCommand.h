#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace presentation {

class PptJumpCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:
	PptJumpCommand(int page, const CString& title);
	virtual ~PptJumpCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace presentation
} // end of namespace commands
} // end of namespace soyokaze

