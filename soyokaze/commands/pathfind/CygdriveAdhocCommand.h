#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace pathfind {



class CydriveAdhocCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:
	CydriveAdhocCommand();
	virtual ~CydriveAdhocCommand();

	CString GetName() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	soyokaze::core::Command* Clone() override;

	static bool IsCygdrivePath(const CString& path);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace soyokaze


