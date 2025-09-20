#pragma once

#include "commands/filter/FilterExecutor.h"

namespace launcherapp { namespace commands { namespace filter {

// キャンセルされたかどうかのフラグ管理
class QueryCancellationToken : public CancellationToken
{
public:
	QueryCancellationToken() = default;
	~QueryCancellationToken() = default;

	// キャンセルが発生したか
	bool IsCancellationRequested() override;
};

}}}

