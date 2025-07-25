#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace unitconvert {


class UnitConvertProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	UnitConvertProvider();
	virtual ~UnitConvertProvider();

public:

	virtual CString GetName();
	// $B0l;~E*$J%3%^%s%I$N=`Hw$r9T$&$?$a$N=i4|2=(B
	void PrepareAdhocCommands() override;
	// $B0l;~E*$J%3%^%s%I$rI,MW$K1~$8$FDs6!$9$k(B
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);
	// Provider$B$,07$&%3%^%s%I<oJL(B($BI=<(L>(B)$B$rNs5s(B
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(UnitConvertProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace unitconvert
} // end of namespace commands
} // end of namespace launcherapp

