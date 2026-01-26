#pragma once

#include "commands/core/CommandProviderIF.h"
#include "utility/RefPtr.h"
#include <stdint.h>


namespace launcherapp {
namespace commands {
namespace common {


class AdhocCommandProviderBase :
	public launcherapp::core::CommandProvider
{
protected:
	using Command = launcherapp::core::Command;
	using Parameter = launcherapp::actions::core::Parameter;

protected:
	AdhocCommandProviderBase();
	virtual ~AdhocCommandProviderBase();

public:
	// 初回起動の初期化を行う
	void OnFirstBoot() override;

	// コマンドの読み込み
	void LoadCommands(CommandFile* commandFile) override;

	CString GetName() override;

	// 作成できるコマンドの種類を表す文字列を取得
	CString GetDisplayName() override;

	// コマンドの種類の説明を示す文字列を取得
	CString GetDescription() override;

	// コマンド新規作成ダイアログ
	bool NewDialog(Parameter* param) override;

	// 非公開コマンドかどうか(新規作成対象にしない)
	bool IsPrivate() const override;

	// 一時的なコマンドの準備を行うための初期化。初回のQueryAdhocCommand前に呼ばれる。
	void PrepareAdhocCommands() override;

	// Provider間の優先順位を表す値を返す。小さいほど優先
	uint32_t GetOrder() const override;

	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	uint32_t AddRef() override;
	uint32_t Release() override;
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

private:
	uint32_t mRefCount;
};


} // end of namespace calculator
} // end of namespace commands
} // end of namespace launcherapp

