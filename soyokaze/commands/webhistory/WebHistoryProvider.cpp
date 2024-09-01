#include "pch.h"
#include "WebHistoryProvider.h"
#include "commands/webhistory/WebHistoryCommand.h"
#include "commands/webhistory/WebHistoryAdhocCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "resource.h"
#include <list>
#include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace webhistory {

using WebHistoryCommandPtr = std::unique_ptr<WebHistoryCommand, std::function<void(void*)> >;
using WebHistoryCommandList = std::vector<WebHistoryCommandPtr>;

using CommandRepository = launcherapp::core::CommandRepository;

struct WebHistoryProvider::PImpl : public launcherapp::core::CommandRepositoryListenerIF
{
	PImpl()
	{
		auto cmdRepo = CommandRepository::GetInstance();
		cmdRepo->RegisterListener(this);

	}
	virtual ~PImpl()
	{
		auto cmdRepo = CommandRepository::GetInstance();
		cmdRepo->UnregisterListener(this);
	}

	void OnDeleteCommand(launcherapp::core::Command* cmd) override
 	{
		for (auto it = mCommands.begin(); it != mCommands.end(); ++it) {
			if ((*it).get() != cmd) {
				continue;
			}

			mCommands.erase(it);
			break;
		}
	}
	void OnLancuherActivate() override {}
	void OnLancuherUnactivate() override {}


	WebHistoryCommandList mCommands;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(WebHistoryProvider)


WebHistoryProvider::WebHistoryProvider() : in(std::make_unique<PImpl>())
{
}

WebHistoryProvider::~WebHistoryProvider()
{
}

static void releaseCmd(void* p)
{
	auto ptr = (WebHistoryCommand*)p;
	if (ptr) {
		ptr->Release();
	}
}


CString WebHistoryProvider::GetName()
{
	return _T("WebHistoryCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString WebHistoryProvider::GetDisplayName()
{
	return _T("ブラウザ履歴コマンド");
}

// コマンドの種類の説明を示す文字列を取得
CString WebHistoryProvider::GetDescription()
{
	static LPCTSTR description = _T("ブラウザ(Edge,Chrome)で閲覧したページを検索するコマンドです\n")
		_T("「コマンド名 (キーワード)」でキーワードを含む過去に閲覧したページを候補として表示します\n")
		_T("検索条件をプリセットできるので、特定の話題に関する履歴だけを抽出する使い方ができます\n")
		_T("");

	return description;
}

// コマンド新規作成ダイアログ
bool WebHistoryProvider::NewDialog(const CommandParameter* param)
{
	std::unique_ptr<WebHistoryCommand> newCmd;
	if (WebHistoryCommand::NewDialog(param, newCmd) == false) {
		return false;
	}

	newCmd->AddRef();  // mCommandsで保持する分の参照カウント+1
	in->mCommands.push_back(WebHistoryCommandPtr(newCmd.get(), releaseCmd));

	bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd.release(), isReloadHotKey);
	return true;
}

// 一時的なコマンドを必要に応じて提供する
void WebHistoryProvider::QueryAdhocCommands(
	Pattern* pattern,
 	std::vector<CommandQueryItem>& commands
)
{
	// 完全一致検索の場合は検索ワード補完をしない
	if (pattern->shouldWholeMatch()) {
		return;
	}

	std::vector<HISTORY> histories;

	CString url;
	CString displayName;
	for (auto& cmd : in->mCommands) {

		if (cmd->QueryHistories(pattern, histories) == false) {
			continue;
		}

		for (auto& history : histories) {
			auto adhocCmd = new WebHistoryAdhocCommand(history);
			commands.push_back(CommandQueryItem(history.mMatchLevel, adhocCmd));
		}
	}
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t WebHistoryProvider::GetOrder() const
{
	return 145;
}

void WebHistoryProvider::OnBeforeLoad()
{
	in->mCommands.clear();
}

bool WebHistoryProvider::LoadFrom(CommandEntryIF* entry, Command** retCommand)
{
	std::unique_ptr<WebHistoryCommand> command(new WebHistoryCommand);
	if (command->Load(entry) == false) {
		return false;
	}
	ASSERT(retCommand);
	*retCommand = command.get();

	command->AddRef();
	in->mCommands.push_back(WebHistoryCommandPtr(command.release(), releaseCmd));

	return true;
}


} // end of namespace webhistory
} // end of namespace commands
} // end of namespace launcherapp

