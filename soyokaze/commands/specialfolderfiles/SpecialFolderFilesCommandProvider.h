#pragma once

#include "commands/common/AdhocCommandProviderBase.h"
#include "commands/core/CommandProviderIF.h"


namespace soyokaze {
namespace commands {
namespace specialfolderfiles {


class SpecialFolderFilesCommandProvider :
	public soyokaze::commands::common::AdhocCommandProviderBase
{
private:
	SpecialFolderFilesCommandProvider();
	virtual ~SpecialFolderFilesCommandProvider();

public:
	CString GetName() override;

	// 一時的なコマンドを必要に応じて提供する
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;


	DECLARE_COMMANDPROVIDER(SpecialFolderFilesCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace soyokaze

