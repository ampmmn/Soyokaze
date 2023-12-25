#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/pathconvert/P4AppSettings.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace pathconvert {


class P4PathConvertAdhocCommand : public soyokaze::commands::common::AdhocCommandBase
{
	using ITEM = P4AppSettings::ITEM;
public:
	P4PathConvertAdhocCommand();
	P4PathConvertAdhocCommand(const ITEM& item);
	virtual ~P4PathConvertAdhocCommand();

	CString GetName() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	soyokaze::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace soyokaze


