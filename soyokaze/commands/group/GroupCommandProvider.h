#pragma once

#include "commands/common/RoutineCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace group {


// グループコマンドを生成するためのクラス
class GroupCommandProvider :
	public launcherapp::commands::common::RoutineCommandProviderBase
{
private:
	GroupCommandProvider();
	~GroupCommandProvider() override;

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

	DECLARE_COMMANDPROVIDER(GroupCommandProvider)

// RoutineCommandProviderBase
	DECLARE_LOADFROM(GroupCommandProvider)
};

} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp

