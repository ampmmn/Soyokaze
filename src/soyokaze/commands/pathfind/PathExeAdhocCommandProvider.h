#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace pathfind {


class PathExeAdhocCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	PathExeAdhocCommandProvider();
	virtual ~PathExeAdhocCommandProvider();

public:
	void AddHistory(const CString& word, const CString& fullPath);

public:
	// $B%3%^%s%I$NFI$_9~$_(B
	virtual void LoadCommands(CommandFile* commandFile);

	virtual CString GetName();
	// $B0l;~E*$J%3%^%s%I$N=`Hw$r9T$&$?$a$N=i4|2=(B
	void PrepareAdhocCommands() override;
	// $B0l;~E*$J%3%^%s%I$rI,MW$K1~$8$FDs6!$9$k(B
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);

	DECLARE_COMMANDPROVIDER(PathExeAdhocCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace launcherapp

