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
	// $B0l;~E*$J%3%^%s%I$rI,MW$K1~$8$FDs6!$9$k(B
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);

	DECLARE_COMMANDPROVIDER(UnitConvertProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace unitconvert
} // end of namespace commands
} // end of namespace launcherapp

