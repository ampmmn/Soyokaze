#pragma once

#include "core/CommandProviderIF.h"


namespace soyokaze {
namespace commands {
namespace activate_window {


class ActivateWindowProvider :
	public soyokaze::core::CommandProvider
{
	using Command = soyokaze::core::Command;
	using CommandParameter = soyokaze::core::CommandParameter;

private:
	ActivateWindowProvider();
	virtual ~ActivateWindowProvider();

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
	virtual void QueryAdhocCommands(Pattern* pattern, std::vector<CommandQueryItem>& comands);

	// Provider間の優先順位を表す値を返す。小さいほど優先
	virtual uint32_t GetOrder() const;

	virtual uint32_t AddRef();
	virtual uint32_t Release();

	DECLARE_COMMANDPROVIDER(ActivateWindowProvider)

protected:
	// Excelシート切り替え用コマンド生成
	void QueryAdhocCommandsForWorksheets(Pattern* pattern, std::vector<CommandQueryItem>& commands);
	// ウインドウ切り替え用コマンド生成
	void QueryAdhocCommandsForWindows(Pattern* pattern, std::vector<CommandQueryItem>& commands);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace activate_window
} // end of namespace commands
} // end of namespace soyokaze

