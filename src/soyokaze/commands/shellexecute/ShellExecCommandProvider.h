#pragma once

#include "commands/common/UserCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace shellexecute {


class ShellExecCommandProvider :
	public launcherapp::commands::common::UserCommandProviderBase
{
private:
	ShellExecCommandProvider();
	~ShellExecCommandProvider() override;

public:
	virtual CString GetName();

	// 作成できるコマンドの種類を表す文字列を取得
	virtual CString GetDisplayName();

	// コマンドの種類の説明を示す文字列を取得
	virtual CString GetDescription();

	// コマンド新規作成ダイアログ
	virtual bool NewDialog(Parameter* param);

	// Provider間の優先順位を表す値を返す。小さいほど優先
	virtual uint32_t GetOrder() const;
	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(ShellExecCommandProvider)

// UserCommandProviderBase
	DECLARE_LOADFROM(ShellExecCommandProvider)
};


}
}
}

