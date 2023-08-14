#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace calculator {



class CalculatorCommand : public soyokaze::commands::common::AdhocCommandBase
{
public:
	CalculatorCommand();
	virtual ~CalculatorCommand();

	void SetResult(const CString& result);

	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

	static bool GetCalcExePath(LPTSTR path, size_t len);
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace calculator
} // end of namespace commands
} // end of namespace soyokaze

