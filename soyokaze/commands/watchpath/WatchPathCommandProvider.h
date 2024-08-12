#pragma once

#include "commands/common/RoutineCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace watchpath {

class WatchPathCommandProvider :
	public launcherapp::commands::common::RoutineCommandProviderBase
{
private:
	WatchPathCommandProvider();
	~WatchPathCommandProvider() override;

public:
	CString GetName() override;

	// 作成できるコマンドの種類を表す文字列を取得
	CString GetDisplayName() override;

	// コマンドの種類の説明を示す文字列を取得
	CString GetDescription() override;

	// コマンド新規作成ダイアログ
	bool NewDialog(const CommandParameter* param) override;

	// Provider間の優先順位を表す値を返す。小さいほど優先
	uint32_t GetOrder() const override;

	DECLARE_COMMANDPROVIDER(WatchPathCommandProvider)

// RoutineCommandProviderBase
	DECLARE_LOADFROM(WatchPathCommandProvider)
};


}
}
}

