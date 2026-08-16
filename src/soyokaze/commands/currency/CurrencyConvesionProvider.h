#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace launcherapp { namespace commands { namespace currency {

class CurrencyConvesionProvider : public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	CurrencyConvesionProvider();
	~CurrencyConvesionProvider() override;

public:
	CString GetName() override;
	void PrepareAdhocCommands() override;
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands) override;
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(CurrencyConvesionProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}}}
