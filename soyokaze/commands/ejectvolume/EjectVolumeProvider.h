#pragma once

#include "commands/common/UserCommandProviderBase.h"


namespace launcherapp {
namespace commands {
namespace ejectvolume {

// リムーバブルドライブを外したりCDドライブのトレイを開くコマンドを生成するためのクラス
class EjectVolumeProvider :
	public launcherapp::commands::common::UserCommandProviderBase
{
private:
	EjectVolumeProvider();
	virtual ~EjectVolumeProvider();

public:
	virtual CString GetName();

	// 作成できるコマンドの種類を表す文字列を取得
	virtual CString GetDisplayName();

	// コマンドの種類の説明を示す文字列を取得
	virtual CString GetDescription();

	// コマンド新規作成ダイアログ
	virtual bool NewDialog(const CommandParameter* param);

	// Provider間の優先順位を表す値を返す。小さいほど優先
	virtual uint32_t GetOrder() const;

	DECLARE_COMMANDPROVIDER(EjectVolumeProvider)

// UserCommandProviderBase
	DECLARE_LOADFROM(EjectVolumeProvider)
};

} // end of namespace ejectvolume
} // end of namespace commands
} // end of namespace launcherapp

