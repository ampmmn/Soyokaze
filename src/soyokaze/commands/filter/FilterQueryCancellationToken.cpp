#include "pch.h"
#include "FilterQueryCancellationToken.h"
#include "commands/core/CommandRepository.h"

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp { namespace commands { namespace filter {

// キャンセルが発生したか?
bool QueryCancellationToken::IsCancellationRequested()
{
	// 後続のコマンド検索が来ていたら、前回のEverything検索を打ち切る
	auto repos = CommandRepository::GetInstance();
	return repos->HasQueryRequest();
}

}}}

