#include "pch.h"
#include "ClipboardHistoryQueryCancellationToken.h"
#include "commands/core/CommandRepository.h"

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp { namespace commands { namespace clipboardhistory {

// キャンセルが発生したか?
bool QueryCancellationToken::IsCancellationRequested()
{
	// 後続のコマンド検索が来ていたら、前回の検索を打ち切る
	auto repos = CommandRepository::GetInstance();
	return repos->HasQueryRequest();
}

}}}

