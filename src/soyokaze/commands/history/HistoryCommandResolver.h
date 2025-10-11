#pragma once

#include "commands/core/CommandIF.h"
#include <memory>

namespace launcherapp { namespace commands { namespace history {

// 履歴として保持しているキーワードに該当するコマンドを取得(解決)するクラス。シングルトン。
class HistoryCommandResolver
{
private:
	HistoryCommandResolver();
	~HistoryCommandResolver();

public:
	// インスタンスを取得する
	static HistoryCommandResolver* GetInstance();
	// キーワードに該当するコマンドを得る
	bool Resolve(const CString& keyword, launcherapp::core::Command** command);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::history

