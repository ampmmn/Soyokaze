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

	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(CalculatorCommand)

	static bool GetCalcExePath(LPTSTR path, size_t len);
public:
	static CString TypeDisplayName();
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace calculator
} // end of namespace commands
} // end of namespace launcherapp

