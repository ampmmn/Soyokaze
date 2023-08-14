#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace recentfiles {


class RecentFileCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:
	RecentFileCommand(const CString& name, const CString& fullPath);
	virtual ~RecentFileCommand();

	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace recentfiles
} // end of namespace commands
} // end of namespace soyokaze

