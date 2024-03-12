#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace simple_dict {

class SimpleDictParam;

class SimpleDictAdhocCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:
public:
	SimpleDictAdhocCommand(const CString& key, const CString& value);
	virtual ~SimpleDictAdhocCommand();

	void SetParam(const SimpleDictParam& param);

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace soyokaze

