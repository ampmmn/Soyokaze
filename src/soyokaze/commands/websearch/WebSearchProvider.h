#pragma once

#include "commands/common/UserCommandProviderBase.h"
#include "setting/AppPreferenceListenerIF.h"

namespace launcherapp {
namespace commands {
namespace websearch {


class WebSearchProvider :
	public launcherapp::commands::common::UserCommandProviderBase,
	public AppPreferenceListenerIF
{
private:
	WebSearchProvider();
	~WebSearchProvider() override;

public:
	CString GetName() override;

	// 作成できるコマンドの種類を表す文字列を取得
	CString GetDisplayName() override;

	// コマンドの種類の説明を示す文字列を取得
	CString GetDescription() override;

	// コマンド新規作成ダイアログ
	bool NewDialog(Parameter* param) override;

	// Provider間の優先順位を表す値を返す。小さいほど優先
	uint32_t GetOrder() const override;
	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(WebSearchProvider)

// UserCommandProviderBase
	void OnBeforeLoad() override;
	bool LoadFrom(CommandEntryIF* entry, Command** command) override;

// AppPreferenceListenerIF
	/** 初回起動時の通知を受け取る */
	void OnAppFirstBoot() override;
	/** 通常起動時の通知を受け取る */
	void OnAppNormalBoot() override;
	/** アプリ設定更新時の通知を受け取る */
	void OnAppPreferenceUpdated() override;
	/** アプリ終了時に専用検索ウインドウを解放する */
	void OnAppExit() override;
};


} // end of namespace websearch
} // end of namespace commands
} // end of namespace launcherapp

