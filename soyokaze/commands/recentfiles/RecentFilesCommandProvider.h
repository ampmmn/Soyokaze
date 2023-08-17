#pragma once

#include "commands/common/AdhocCommandProviderBase.h"
#include "core/CommandProviderIF.h"


namespace soyokaze {
namespace commands {
namespace recentfiles {


class RecentFilesCommandProvider :
	public soyokaze::commands::common::AdhocCommandProviderBase
{
private:
	RecentFilesCommandProvider();
	virtual ~RecentFilesCommandProvider();

public:
	CString GetName() override;

	// 一時的なコマンドを必要に応じて提供する
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;


	DECLARE_COMMANDPROVIDER(RecentFilesCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace recentfiles
} // end of namespace commands
} // end of namespace soyokaze

