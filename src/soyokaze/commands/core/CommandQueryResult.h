#pragma once

#include "commands/core/CommandIF.h"

namespace launcherapp { namespace commands { namespace core {

	// コマンド検索結果のためのインタフェース
class CommandQueryResult
{
public:
	// 検索にヒットしたコマンド数を取得する
	virtual size_t GetCount() = 0;
	// 検索結果数が0かどうかを取得する
	virtual bool IsEmpty() = 0;
	// 結果を取得する(参照カウントは呼び出し元で-1する必要あり)
	virtual bool Get(size_t index, launcherapp::core::Command** cmd, int* matchLevel) = 0;
	// 結果を取得する(参照カウントは呼び出し元で-1する必要あり)
	virtual launcherapp::core::Command* GetItem(size_t index, int* matchLevel = nullptr) = 0;
	// 参照カウントを上げる
	virtual uint32_t AddRef() = 0;
	// 参照カウントを下げる
	virtual uint32_t Release() = 0;
};

}}}

