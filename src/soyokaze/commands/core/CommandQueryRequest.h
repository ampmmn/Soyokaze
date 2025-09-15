#pragma once

#include "commands/core/CommandQueryResult.h"

namespace launcherapp { namespace commands { namespace core {

// コマンドの問い合わせ要求のためのインタフェース
class CommandQueryRequest
{
public:
	virtual ~CommandQueryRequest() {}

	// 検索キーワード(文字列全体)を取得する
	virtual CString GetCommandParameter() = 0;
	// 検索結果を通知するためのコールバック関数
	virtual void NotifyQueryComplete(bool isCancelled, CommandQueryResult* result) = 0;
	// 参照カウントを上げる
	virtual uint32_t AddRef() = 0;
	// 参照カウントを下げる
	virtual uint32_t Release() = 0;
};

}}}

