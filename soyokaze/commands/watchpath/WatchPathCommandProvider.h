#pragma once

#include "commands/core/CommandProviderIF.h"

namespace soyokaze {
namespace commands {
namespace watchpath {


class WatchPathCommandProvider :
	public soyokaze::core::CommandProvider
{
	using Command = soyokaze::core::Command;
	using CommandParameter = soyokaze::core::CommandParameter;

private:
	WatchPathCommandProvider();
	virtual ~WatchPathCommandProvider();

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
	virtual bool NewDialog(const CommandParameter* param);

	// 非公開コマンドかどうか(新規作成対象にしない)
	virtual bool IsPrivate() const;

	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);

	// Provider間の優先順位を表す値を返す。小さいほど優先
	virtual uint32_t GetOrder() const;

	// 設定ページを取得する
	bool CreateSettingPages(CWnd* parent, std::vector<SettingPage*>& pages) override;

	virtual uint32_t AddRef();
	virtual uint32_t Release();

	DECLARE_COMMANDPROVIDER(WatchPathCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}
}

