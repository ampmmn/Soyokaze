#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace simple_dict {

class SimpleDictParam;

class SimpleDictAdhocCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
public:
	SimpleDictAdhocCommand(const SimpleDictParam& param, const CString& key, const CString& value);
	virtual ~SimpleDictAdhocCommand();

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


} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

