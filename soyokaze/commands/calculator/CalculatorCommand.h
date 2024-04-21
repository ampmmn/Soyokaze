#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace calculator {



class CalculatorCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	CalculatorCommand();
	CalculatorCommand(int base);
	virtual ~CalculatorCommand();

	void SetResult(const CString& result);

	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	static bool GetCalcExePath(LPTSTR path, size_t len);
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace calculator
} // end of namespace commands
} // end of namespace launcherapp

