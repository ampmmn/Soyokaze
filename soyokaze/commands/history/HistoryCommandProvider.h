#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace history {


/**
 *  履歴表示のためのコマンドオブジェクト生成クラス
 */
class HistoryCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	HistoryCommandProvider();
	virtual ~HistoryCommandProvider();

public:
	void AddHistory(const CString& word, const CString& fullPath);

public:
	virtual CString GetName();
	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);

	DECLARE_COMMANDPROVIDER(HistoryCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace history
} // end of namespace commands
} // end of namespace launcherapp

