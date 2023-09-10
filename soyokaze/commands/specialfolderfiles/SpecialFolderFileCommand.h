#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/specialfolderfiles/SpecialFolderFile.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace specialfolderfiles {


class SpecialFolderFileCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:
	SpecialFolderFileCommand(const ITEM& item);
	virtual ~SpecialFolderFileCommand();

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace soyokaze

