#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp { namespace commands { namespace currency {

class CurrencyConvesionCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	CurrencyConvesionCommand(double value, const CString& currency);
	~CurrencyConvesionCommand() override;

	CString GetName() override;
	CString GetDescription() override;
	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(CurrencyConvesionCommand)

public:
		static CString TypeDisplayName();

private:
		struct PImpl;
		std::unique_ptr<PImpl> in;
};

}}}
