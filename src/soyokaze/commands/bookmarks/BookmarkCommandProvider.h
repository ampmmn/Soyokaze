#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace bookmarks {

class BookmarkCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	BookmarkCommandProvider();
	~BookmarkCommandProvider() override;

public:
	CString GetName() override;

	// $B0l;~E*$J%3%^%s%I$N=`Hw$r9T$&$?$a$N=i4|2=(B
	void PrepareAdhocCommands() override;
	// $B0l;~E*$J%3%^%s%I$rI,MW$K1~$8$FDs6!$9$k(B
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;
	// Provider$B$,07$&%3%^%s%I<oJL(B($BI=<(L>(B)$B$rNs5s(B
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(BookmarkCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

