#pragma once

#include "commands/common/RoutineCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace volumecontrol {

// 音量調節コマンドを生成するためのクラス
class VolumeCommandProvider :
	public launcherapp::commands::common::RoutineCommandProviderBase
{
private:
	VolumeCommandProvider();
	~VolumeCommandProvider() override;

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

	DECLARE_COMMANDPROVIDER(VolumeCommandProvider)

// RoutineCommandProviderBase
	DECLARE_LOADFROM(ShellExecCommandProvider)
};

} // end of namespace volumecontrol
} // end of namespace commands
} // end of namespace launcherapp

