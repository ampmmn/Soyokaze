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
	CString GetName() override;

	// 一時的なコマンドの準備を行うための初期化。初回のQueryAdhocCommand前に呼ばれる。
	void PrepareAdhocCommands() override;
	// 一時的なコマンドを必要に応じて提供する
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;

	DECLARE_COMMANDPROVIDER(HistoryCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace history
} // end of namespace commands
} // end of namespace launcherapp

