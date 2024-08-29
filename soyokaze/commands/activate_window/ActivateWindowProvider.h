#pragma once

#include "commands/common/UserCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace activate_window {


class ActivateWindowProvider :
	public launcherapp::commands::common::UserCommandProviderBase
{
private:
	ActivateWindowProvider();
	~ActivateWindowProvider() override;

public:
	CString GetName() override;

	// 作成できるコマンドの種類を表す文字列を取得
	CString GetDisplayName() override;

	// コマンドの種類の説明を示す文字列を取得
	CString GetDescription() override;

	// コマンド新規作成ダイアログ
	bool NewDialog(const CommandParameter* param) override;

	// 一時的なコマンドを必要に応じて提供する
	void QueryAdhocCommands(Pattern* pattern, std::vector<CommandQueryItem>& comands) override;

	// Provider間の優先順位を表す値を返す。小さいほど優先
	uint32_t GetOrder() const override;

	DECLARE_COMMANDPROVIDER(ActivateWindowProvider)

// UserCommandProviderBase
	DECLARE_LOADFROM(ActivateWindowProvider)
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
} // end of namespace launcherapp

