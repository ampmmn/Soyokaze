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
	using CommandParameter = launcherapp::core::CommandParameter;

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
	bool NewDialog(CommandParameter* param) override;

	// 非公開コマンドかどうか(新規作成対象にしない)
	bool IsPrivate() const override;

	// Provider間の優先順位を表す値を返す。小さいほど優先
	uint32_t GetOrder() const override;

	// 設定ページを取得する
	bool CreateSettingPages(CWnd* parent, std::vector<SettingPage*>& pages) override;

	uint32_t AddRef() override;
	uint32_t Release() override;

private:
	uint32_t mRefCount;
};


} // end of namespace calculator
} // end of namespace commands
} // end of namespace launcherapp

