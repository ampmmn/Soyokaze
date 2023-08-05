#pragma once

#include "commands/common/AdhocCommandProviderBase.h"


namespace soyokaze {
namespace commands {
namespace activate_window {


class ActivateWindowProvider :
	public soyokaze::commands::common::AdhocCommandProviderBase
{
private:
	ActivateWindowProvider();
	virtual ~ActivateWindowProvider();

public:
	virtual CString GetName();

	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, std::vector<CommandQueryItem>& comands);


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

