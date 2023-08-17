#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace soyokaze {
namespace commands {
namespace pathfind {


class PathExeAdhocCommandProvider :
	public soyokaze::commands::common::AdhocCommandProviderBase
{
private:
	PathExeAdhocCommandProvider();
	virtual ~PathExeAdhocCommandProvider();

public:
	void AddHistory(const CString& word, const CString& fullPath);

public:
	// コマンドの読み込み
	virtual void LoadCommands(CommandFile* commandFile);

	virtual CString GetName();
	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);

	DECLARE_COMMANDPROVIDER(PathExeAdhocCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace soyokaze

