#pragma once

#include "commands/core/CommandProviderIF.h"

namespace launcherapp {
namespace commands {
namespace builtin {


class BuiltinCommandProvider :
	public launcherapp::core::CommandProvider
{
	using Command = launcherapp::core::Command;
	using Parameter = launcherapp::actions::core::Parameter;

private:
	BuiltinCommandProvider();
	virtual ~BuiltinCommandProvider();

public:
	// 初回起動の初期化を行う
	virtual void OnFirstBoot();

	// コマンドの読み込み
	virtual void LoadCommands(CommandFile* commandFile);

	virtual CString GetName();

	// 作成できるコマンドの種類を表す文字列を取得
	virtual CString GetDisplayName();

	// コマンドの種類の説明を示す文字列を取得
	virtual CString GetDescription();

	// コマンド新規作成ダイアログ
	virtual bool NewDialog(Parameter* param);

	// 非公開コマンドかどうか(新規作成対象にしない)
	virtual bool IsPrivate() const;

	// 一時的なコマンドの準備を行うための初期化。初回のQueryAdhocCommand前に呼ばれる。
	void PrepareAdhocCommands() override;

	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);

	// Provider間の優先順位を表す値を返す。小さいほど優先
	virtual uint32_t GetOrder() const;

	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	uint32_t AddRef() override;
	uint32_t Release() override;
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_COMMANDPROVIDER(BuiltinCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

